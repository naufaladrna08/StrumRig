#ifndef AUDIOENGINE_HPP
#define AUDIOENGINE_HPP

#include <pipewire/pipewire.h>
#include <functional>
#include <memory>
#include <string>
#include <cstdlib>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)
#define LOG(...) fprintf(stdout, __VA_ARGS__)

class AudioEngine {
  public:
    AudioEngine();
    ~AudioEngine();

    bool initialized();
    void run();
    void stop();
    void startThread();
    void stopThread();

    void setInputCallback(std::function<void(const float*, size_t)> callback);
    void setOutputCallback(std::function<void(float*, size_t)> callback);

    typedef struct {
      std::string name;
      uint32_t index;
    } Device;

   struct Impl {
    pw_main_loop *mainLoop;
    pw_context *context;
    pw_stream *stream;
    pw_core *core;
    pw_registry *registry;
    std::function<void(const float*, size_t)> inputCallback;
    std::function<void(float*, size_t)> outputCallback;
    std::vector<AudioEngine::Device> devices;

    static void onProcess(void* userdata);
    static void onGlobalAdded(void *data, uint32_t id, uint32_t permissions, const char *type, uint32_t version, const struct spa_dict *props);
    void setupStream();
    void addDevice(const AudioEngine::Device& device);
    std::vector<AudioEngine::Device> getDevices();

    static const struct pw_registry_events registerEvents;
    static const struct pw_stream_events streamEvents;
    struct spa_hook registerListener;
    std::mutex mutex;
  };

  private:
    std::unique_ptr<Impl> m_impl;
    
    std::atomic<bool> m_running;
    std::thread m_thread;

  public:
    std::unique_ptr<Impl> GetImplementation();
};

#endif