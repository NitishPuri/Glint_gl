#pragma once

#include "Window.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class ImGuiLayer {
 public:
  ImGuiLayer(Window& window) : m_Window(window) {}
  void init() {
    // Setup ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_Window.getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
  }
  ~ImGuiLayer() {}

  void beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

 private:
  Window& m_Window;
};