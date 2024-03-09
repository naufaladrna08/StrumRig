#define DEBUG
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>
#include <algorithm>
#include "GraphEditor.hpp"
#include "Theme.hpp"
#include "AudioEngine.hpp"

typedef struct {
  bool is
} stateAction;

static STATE state;

static GraphEditorImpl delegate;
bool GraphEditorImpl::isContextMenuOpen = false;
static bool shouldOpenPopup = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  // CTRL + A
  if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_A && action == GLFW_PRESS) {
    shouldOpenPopup = true;
  }
}

int main(int argc, char** argv) {
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }  

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL", NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glfwSetKeyCallback(window, key_callback);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  ImGuiTheme();
  io.Fonts->AddFontFromFileTTF("../assets/fonts/Ubuntu-Medium.ttf", 16.0f);

  bool p_open = true;
  double mouseX, mouseY;
  glfwGetCursorPos(window, &mouseX, &mouseY);
    
  static GraphEditor::Options options;
  static GraphEditor::ViewState viewState;
  static GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
  static bool showGraphEditor = true;

  AudioEngine engine;
  if (!engine.initialized()) {
    std::cout << "Failed to initialize audio engine" << std::endl;
    return -1;
  }

  engine.startThread();

  // Input and Output
  delegate.addNode("Input", 0, 100, 100);
  delegate.addNode("Output", 1, 500, 100);

  // engine.GetImplementation()->getDevices();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
      ImGui::PopStyleVar();

      ImGuiID dockspace_id = ImGui::GetID("dock");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("New")) {}
          if (ImGui::MenuItem("Open")) {}
          if (ImGui::MenuItem("Save")) {}
          if (ImGui::MenuItem("Save As")) {}
          if (ImGui::MenuItem("Exit")) {}
          ImGui::End();
        }

        ImGui::EndMenuBar();
      }

      if (shouldOpenPopup) {
        ImGui::OpenPopup("Node");
        shouldOpenPopup = false; // Reset the flag
      }

      if (ImGui::BeginPopupModal("Node", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("This is a modal with a textbox!");

        // Textbox
        std::string text = "Enter text here";
        ImGui::InputText("##textbox", &text);

        // You can process the text input here (text variable)

        // Buttons to close the modal
        if (ImGui::Button("OK")) {
          delegate.addNode(text.c_str(), 0, mouseX, mouseY);
          ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
          // Cancel button action here

          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }

      // ImGui::ShowDemoWindow();

      if (showGraphEditor) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::Begin("Effect Rack", NULL, flags);

        GraphEditor::Show(delegate, options, viewState, true, &fit);

        ImGui::End();
      }

      if (delegate.isContextMenuOpen) {
        ImGui::OpenPopup("Context Menu");
        delegate.isContextMenuOpen = false;
      }

      if (ImGui::BeginPopup("Context Menu")) {
        if (ImGui::MenuItem("Add Input")) {
          delegate.addNode("New Node", 2, 0, 0);
        }
        
        if (ImGui::MenuItem("Add Output")) {
          delegate.addNode("New Node", 1, 0, 0);
        }

        ImGui::EndPopup();
      }

    ImGui::End();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  engine.stopThread();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
