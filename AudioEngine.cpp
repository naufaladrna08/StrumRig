#include "AudioEngine.hpp"

void AudioEngine::Impl::onProcess(void* userdata) {
  auto engine = static_cast<AudioEngine::Impl*>(userdata);

  pw_buffer *buffer = pw_stream_dequeue_buffer(engine->stream);
  if (!buffer) {
    LOG_ERR("Failed to dequeue buffer\n");
    return;
  }

  spa_buffer *spaBuffer = buffer->buffer;
  if (spaBuffer->datas[0].data) {
    float* input = static_cast<float*>(spaBuffer->datas[0].data);
    size_t numSamples = spaBuffer->datas[0].chunk->size / sizeof(float);
    if (engine->inputCallback) {
      engine->inputCallback(input, numSamples);
    }

    if (engine->outputCallback) {
      engine->outputCallback(input, numSamples);
    }
  }

  pw_stream_queue_buffer(engine->stream, buffer);
}

void AudioEngine::Impl::onGlobalAdded(void *data, uint32_t id, uint32_t permissions, const char *type, uint32_t version, const struct spa_dict *props) {
  auto engine = static_cast<AudioEngine::Impl*>(data);

  if (props == nullptr) {
    LOG_ERR("Failed to get properties\n");
    return;
  }
  
  if (strstr(type, "PipeWire:Interface:Node") != NULL) {
    // We've found a node, now check if it's an audio device
    const char *media_class = spa_dict_lookup(props, "media.class");
    if (media_class != NULL && (strcmp(media_class, "Audio/Source") == 0 || strcmp(media_class, "Audio/Sink") == 0)) {
      const char *name = spa_dict_lookup(props, "node.name"); // node.name is commonly used, but you can check for others
      if (!name) { // Fallbacks or other properties if node.name isn't available
        name = spa_dict_lookup(props, "media.name");
      }

      if (!name) {
        name = spa_dict_lookup(props, "device.description");
      }
      // If no suitable name was found, use a generic placeholder
      if (!name) {
        name = "Unknown Audio Object";
      }

      LOG("Found audio device: %s\n", name);

      AudioEngine::Device device { name, id };
      engine->addDevice(device);
    }
  }
}

void AudioEngine::Impl::setupStream() {
  pw_stream* stream = pw_stream_new_simple(pw_main_loop_get_loop(mainLoop), "audio-engine", 
    pw_properties_new(
      PW_KEY_MEDIA_CLASS, "Audio/Sink",
      PW_KEY_MEDIA_ROLE, "Music",
      NULL
    ),
    &streamEvents,
    this
  );
}

void AudioEngine::Impl::addDevice(const AudioEngine::Device& device) {
  mutex.lock();
  devices.push_back(device);
  mutex.unlock();
}

std::vector<AudioEngine::Device> AudioEngine::Impl::getDevices() {
  std::lock_guard<std::mutex> lock(mutex);
  #if DEBUG
  for (const auto& device : devices) {
    LOG("Device: %s\n", device.name.c_str());
  }
  #endif
  
  return devices;
}

const struct pw_registry_events AudioEngine::Impl::registerEvents = {
  PW_VERSION_REGISTRY_EVENTS,
  .global = AudioEngine::Impl::onGlobalAdded
};

const struct pw_stream_events AudioEngine::Impl::streamEvents = {
  PW_VERSION_STREAM_EVENTS,
  .process = AudioEngine::Impl::onProcess
};

AudioEngine::AudioEngine() 
: m_impl(std::make_unique<Impl>()) {

}

AudioEngine::~AudioEngine() {
  if (m_impl->stream) {
    pw_stream_destroy(m_impl->stream);
  }

  if (m_impl->context) {
    pw_context_destroy(m_impl->context);
  }

  if (m_impl->mainLoop) {
    pw_main_loop_destroy(m_impl->mainLoop);
  }
}

bool AudioEngine::initialized() {
  pw_init(nullptr, nullptr);
  m_impl->mainLoop = pw_main_loop_new(NULL);
  if (m_impl->mainLoop == NULL) {
    LOG_ERR("Failed to create main loop\n");
    return false;
  }
  
  LOG("Pipewire %s\n", pw_get_library_version());
  
  m_impl->context = pw_context_new(pw_main_loop_get_loop(m_impl->mainLoop), NULL, 0);
  m_impl->core = pw_context_connect(m_impl->context, NULL, 0);
  m_impl->registry = pw_core_get_registry(m_impl->core, PW_VERSION_REGISTRY, 0);
  
  spa_zero(m_impl->registerListener);
  pw_registry_add_listener(m_impl->registry, &m_impl->registerListener, &m_impl->registerEvents, m_impl.get());
  
  return true;
}

void AudioEngine::run() {
  LOG("Running audio engine\n");
  pw_main_loop_run(m_impl->mainLoop);
}

void AudioEngine::stop() {
  LOG("Stopping audio engine\n");
  pw_main_loop_quit(m_impl->mainLoop);
}

void AudioEngine::startThread() {
  m_running = true;
  m_thread = std::thread([this] {
    run();
  });
}

void AudioEngine::stopThread() {
  stop();

  m_running = false;
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void AudioEngine::setInputCallback(std::function<void(const float*, size_t)> callback) {
  m_impl->inputCallback = callback;
}

void AudioEngine::setOutputCallback(std::function<void(float*, size_t)> callback) {
  m_impl->outputCallback = callback;
}

std::unique_ptr<AudioEngine::Impl> AudioEngine::GetImplementation() {
  return std::move(m_impl);
}