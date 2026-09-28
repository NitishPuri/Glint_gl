#pragma once

#include "Core/Renderer.h"
#include "glm/glm.hpp"

struct CameraProps {
  float fov, aspect, near, far;
  glm::vec3 position, target, up;
};

inline CameraProps getDefaultCameraProps() {
  CameraProps props;
  props.fov = 45.0f;
  props.aspect = 4.0f / 3.0f;
  props.near = 0.1f;
  props.far = 100.0f;
  props.position = glm::vec3(4, 3, -3);
  props.target = glm::vec3(0, 0, 0);
  props.up = glm::vec3(0, 1, 0);
  return props;
}

glm::mat4 getViewProjectionMatrix(const CameraProps& props);

enum CameraMode {
  CAMERA_MODE_NONE,
  CAMERA_MODE_FREE,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ARCBALL,
};

class CameraController {
 public:
  CameraController(const CameraProps& props) : m_Props(props) { m_ViewProjection = getViewProjectionMatrix(m_Props); }

  void update(float deltaTime);

  glm::mat4 getViewProjection() const { return m_ViewProjection; }
  glm::mat4 getProjectionMatrix() const;
  glm::mat4 getViewMatrix() const;

  CameraProps& getProps() { return m_Props; }
  CameraMode m_Mode = CAMERA_MODE_ARCBALL;

  void onImGuiRender();

 private:
  void processInputs(float dt);

  CameraProps m_Props;
  glm::mat4 m_ViewProjection;

  float m_speed = 1.f;
};