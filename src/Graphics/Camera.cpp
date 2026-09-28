#include "Camera.h"

#include <algorithm>

#include "glm/gtc/matrix_transform.hpp"

#define ARCBALL_CAMERA_IMPLEMENTATION
#include "arcball_camera/arcball_camera.h"

glm::mat4 getViewProjectionMatrix(const CameraProps& props) {
  glm::mat4 Projection = glm::perspective(glm::radians(props.fov), props.aspect, props.near, props.far);
  glm::mat4 View = glm::lookAt(props.position, props.target, props.up);
  return Projection * View;
}

void CameraController::update(float deltaTime) {
  processInputs(deltaTime);
  m_ViewProjection = getViewProjectionMatrix(m_Props);
}

glm::mat4 CameraController::getProjectionMatrix() const {
  return glm::perspective(glm::radians(m_Props.fov), m_Props.aspect, m_Props.near, m_Props.far);
}

glm::mat4 CameraController::getViewMatrix() const { return glm::lookAt(m_Props.position, m_Props.target, m_Props.up); }

void CameraController::processInputs(float dt) {
  ImGuiIO& io = ImGui::GetIO();

  // update aspect ratio

  m_Props.aspect = io.DisplaySize.x / io.DisplaySize.y;
  // Logger::logf("Display Size: {}, aspect, {} ", io.DisplaySize, m_Props.aspect);
  ImVec2 mouse_drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f);
  ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

  //   if (!(io.MouseDown[1] || io.MouseDown[2] || (fabs(io.MouseWheel) > 0.01f))) {
  //     return;
  //   }

  static int cntr = 0;
  cntr++;

  auto speed = 0.1f;
  auto dx = mouse_drag.x * speed;
  auto dy = mouse_drag.y * speed;

  if (m_Mode == CAMERA_MODE_FREE) {
    glm::vec3 forward = m_Props.target - m_Props.position;
    glm::vec3 right = glm::normalize(glm::cross(forward, m_Props.up));

    m_Props.position += forward * dy + right * dx;
    m_Props.target += forward * dy + right * dx;

  } else if (m_Mode == CAMERA_MODE_ORBIT) {
    glm::vec3 direction = m_Props.position - m_Props.target;
    auto radius = glm::length(direction);

    auto theta = atan2(direction.z, direction.x);
    auto phi = acos(direction.y / radius);

    // TODO: arcball_camera_update is great, make better use of it with keyboard, remove other two modes

    theta += dx;
    phi = std::clamp(phi + dy, 0.1f, 3.13f);

    direction[0] = radius * sin(phi) * cos(theta);
    direction[1] = radius * cos(phi);
    direction[2] = radius * sin(phi) * sin(theta);
    m_Props.position = m_Props.target + direction;

  } else if (m_Mode == CAMERA_MODE_ARCBALL) {
    float eye[3] = {m_Props.position.x, m_Props.position.y, m_Props.position.z};
    float target[3] = {m_Props.target.x, m_Props.target.y, m_Props.target.z};
    float up[3] = {m_Props.up.x, m_Props.up.y, m_Props.up.z};

    // float view[16];  // TODO: Can just use this, instead of recalculating view matrix.

    arcball_camera_update(eye, target, up, nullptr,                      //
                          dt, m_speed, m_speed, m_speed,                 //
                          int(io.DisplaySize.x), int(io.DisplaySize.y),  //
                          int(io.MousePosPrev.x), int(io.MousePos.x),    //
                          int(io.MousePosPrev.y), int(io.MousePos.y),    //
                          io.MouseDown[2], io.MouseDown[1], int(io.MouseWheel), 0);
    m_Props.position = glm::vec3(eye[0], eye[1], eye[2]);
    m_Props.target = glm::vec3(target[0], target[1], target[2]);
    m_Props.up = glm::vec3(up[0], up[1], up[2]);
  }
}

void CameraController::onImGuiRender() {
  ImGui::Begin("Camera Control Panel");

  ImGui::Text("Camera Manip :: ");
  ImGui::SameLine();
  if (ImGui::RadioButton("None", m_Mode == CAMERA_MODE_NONE)) {
    m_Mode = CAMERA_MODE_NONE;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Free Mode", m_Mode == CAMERA_MODE_FREE)) {
    m_Mode = CAMERA_MODE_FREE;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Orbit Mode", m_Mode == CAMERA_MODE_ORBIT)) {
    m_Mode = CAMERA_MODE_ORBIT;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Arcball Mode", m_Mode == CAMERA_MODE_ARCBALL)) {
    m_Mode = CAMERA_MODE_ARCBALL;
  }

  ImGui::SliderFloat("speed", &m_speed, 0.2f, 2.0f);

  ImGui::SliderFloat("FOV", &m_Props.fov, 10.0f, 179.0f);
  ImGui::SliderFloat("Near", &m_Props.near, 0.01f, 1.0f);
  ImGui::SliderFloat("Far", &m_Props.far, 100.f, 10000.0f);

  ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", m_Props.position.x, m_Props.position.y, m_Props.position.z);
  ImGui::Text("Camera Target: (%.2f, %.2f, %.2f)", m_Props.target.x, m_Props.target.y, m_Props.target.z);

  ImGui::End();
}