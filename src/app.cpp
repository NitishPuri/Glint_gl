#include <chrono>
#include <iostream>
#include <memory>

#include "Core/GuiLayer.h"
#include "Core/Logger.h"
#include "Core/SceneManager.h"
#include "Core/Window.h"
#include "Graphics/Shader.h"

//
#include "Scenes/BasicShading.h"
#include "Scenes/CubeScene.h"
#include "Scenes/NormalMapping.h"
#include "Scenes/QuadScene.h"
#include "Scenes/RenderToTexture.h"
#include "Scenes/ShadowMapping.h"
#include "Scenes/UVCubeScene.h"
#include "Scenes/VBOIndexing.h"

const int WINDOW_WIDTH = 1800;
const int WINDOW_HEIGHT = 1600;

class Application {
 public:
  Application() : m_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Glint"), m_ImGuiLayer(m_Window) {}
  ~Application() {}

  void init() {
    Logger::log("Initializing application...");
    // m_CurrentScene = std::make_shared<QuadScene>();
    if (m_Window.init() == -1) {
      Logger::error("Failed to initialize window");
      exit(EXIT_FAILURE);
    }

    m_Window.setSceneManager(&m_SceneManager);

    m_ImGuiLayer.init();

    Logger::log("Registering scenes...");
    // register scenes
    // TODO: This should be done at the place of scene definition somehow,
    // a singleton registry or something for scenes?
    m_SceneManager.registerScene("1_quad", []() { return std::make_shared<QuadScene>(); });
    m_SceneManager.registerScene("2_cube", []() { return std::make_shared<CubeScene>(); });
    m_SceneManager.registerScene("3_cube_uv", []() { return std::make_shared<UVCubeScene>(); });
    m_SceneManager.registerScene("4_basic_shading", []() { return std::make_shared<BasicShading>(); });
    m_SceneManager.registerScene("5_vbo_indexing", []() { return std::make_shared<VBOIndexing>(); });
    m_SceneManager.registerScene("6_normal_mapping", []() { return std::make_shared<NormalMapping>(); });
    m_SceneManager.registerScene("7_render_to_texture", []() { return std::make_shared<RenderToTexture>(); });
    m_SceneManager.registerScene("8_shadow_mapping", []() { return std::make_shared<ShadowMapping>(); });
  }

  void run() {
    Logger::log("Running application...");

    // Load initial scene
    m_SceneManager.loadScene("8_shadow_mapping");

    auto last_time = std::chrono::high_resolution_clock::now();

    while (!m_Window.shouldClose()) {
      m_Window.pollEvents();

      auto current_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> delta_time = current_time - last_time;
      last_time = current_time;

      // Start ImGui frame
      m_ImGuiLayer.beginFrame();

      m_SceneManager.onUpdate(delta_time.count());
      m_SceneManager.onRender();
      m_SceneManager.onImGuiRender();

      // Render ImGui
      m_ImGuiLayer.endFrame();

      m_Window.swapBuffers();
    }
  }

 private:
  Window m_Window;
  ImGuiLayer m_ImGuiLayer;
  SceneManager m_SceneManager;
};

int main() {
  Logger::init("./log.txt");
  Logger::log("Starting application");
  Application app;
  app.init();

  app.run();

  return 0;
}
