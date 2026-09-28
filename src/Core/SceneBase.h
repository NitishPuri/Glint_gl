#pragma once

#include <string>

class SceneBase {
 public:
  SceneBase(const std::string& name) : m_Name(name) {}
  virtual ~SceneBase() = default;

  virtual void onAttach() {}                                  // Called when demo is selected
  virtual void onDetach() {}                                  // Called when switching to another demo
  virtual void onUpdate([[maybe_unused]] float deltaTime) {}  // For logic updates
  virtual void onRender() = 0;                                // For rendering
  virtual void onImGuiRender() {}                             // For demo-specific controls

  virtual void onWindowResize([[maybe_unused]] int width, [[maybe_unused]] int height) {}

  const std::string& getName() const { return m_Name; }

 protected:
  std::string m_Name;
};