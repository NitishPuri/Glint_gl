#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "Renderer.h"
#include "SceneBase.h"

class SceneManager {
 public:
  SceneManager() {}
  ~SceneManager() {}

  void registerScene(const std::string& name, std::function<std::shared_ptr<SceneBase>()> scene) {
    m_Scenes[name] = scene;
  }

  void loadScene(const std::string& name) {
    if (m_CurrentScene) {
      m_CurrentScene->onDetach();
    }
    if (name.empty()) {
      m_CurrentScene = nullptr;
      return;
    }
    auto it = m_Scenes.find(name);
    if (it != m_Scenes.end()) {
      // create scene
      m_CurrentScene = it->second();
      m_CurrentScene->onAttach();
    }
  }

  void onUpdate(float deltaTime) {
    // Logger::log("SceneManager::onUpdate : ", deltaTime);
    if (m_CurrentScene) {
      m_CurrentScene->onUpdate(deltaTime);
    }
  }

  void onRender() {
    if (m_CurrentScene)
      m_CurrentScene->onRender();
    else {
      // clear the screen
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
    }
  }

  void renderingUI() {
    if (ImGui::Checkbox("Wireframe", &RendererConfig::m_Wireframe)) {
      glPolygonMode(GL_FRONT_AND_BACK, RendererConfig::m_Wireframe ? GL_LINE : GL_FILL);
    }
    ImGui::SameLine();

    if (ImGui::Checkbox("Depth Test", &RendererConfig::m_DepthTest)) {
      if (RendererConfig::m_DepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    ImGui::SameLine();

    // if (ImGui::Checkbox("Cull Face", &m_CullFace)) {
    //   if (m_CullFace) {
    //     glEnable(GL_CULL_FACE);
    //   } else {
    //     glDisable(GL_CULL_FACE);
    //   }
    // }

    if (ImGui::Checkbox("Blend", &RendererConfig::m_Blend)) {
      if (RendererConfig::m_Blend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glDisable(GL_BLEND);
      }
    }
    ImGui::SameLine();

    ImGui::Checkbox("Show Stats", &RendererConfig::m_ShowStats);
    if (RendererConfig::m_ShowStats) {
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices,
                  io.MetricsRenderIndices / 3);
    }
  }

  void onImGuiRender() {
    ImGui::Begin("Scene Control Panel");
    if (ImGui::BeginCombo("Scene", m_CurrentScene ? m_CurrentScene->getName().c_str() : "Select Scene")) {
      for (const auto& pair : m_Scenes) {
        bool is_selected = (m_CurrentScene && m_CurrentScene->getName() == pair.first);
        if (ImGui::Selectable(pair.first.c_str(), is_selected)) {
          loadScene(pair.first);
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      if (ImGui::Selectable("<none>", m_CurrentScene == nullptr)) {
        loadScene("");
      }

      ImGui::EndCombo();
    }

    renderingUI();

    ImGui::End();

    if (m_CurrentScene) {
      m_CurrentScene->onImGuiRender();
    }
  }

  void onWindowResize(int width, int height) {
    if (m_CurrentScene) {
      m_CurrentScene->onWindowResize(width, height);
    }
  }

 private:
  std::unordered_map<std::string, std::function<std::shared_ptr<SceneBase>()>> m_Scenes;
  std::shared_ptr<SceneBase> m_CurrentScene;
};