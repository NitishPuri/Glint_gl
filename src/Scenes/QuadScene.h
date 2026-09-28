#pragma once

// core
#include "Core/SceneBase.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/VertexArray.h"
#include "Graphics/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class QuadScene : public SceneBase {
 public:
  QuadScene() : SceneBase("Quad Scene") {}
  void onAttach() override {
    glDisable(GL_DEPTH_TEST);
    m_Shader.init(getFilePath("/shaders/quad.vert"), getFilePath("/shaders/quad.frag"));

    // Quad Data
    // x, y, r, g, b
    float vertices[] = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // Bottom-left (red)
        0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom-right (green)
        0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Top-right (blue)
        -0.5f, 0.5f,  1.0f, 1.0f, 0.0f   // Top-left (yellow)
    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices, 6);

    // Position attribute
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0));
    GLCall(glEnableVertexAttribArray(0));

    // Color attribute
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(1));
  }

  void onDetach() override {
    m_VertexBuffer.reset();
    m_IndexBuffer.reset();
    m_VertexArray.reset();
  };

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += m_RotationSpeed * deltaTime;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;
  };

  void onRender() override {
    // Clear screen
    GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::rotate(transform, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // Render Quad
    m_Shader.bind();
    m_Shader.setUniformMat4("transform", transform);

    m_VertexArray->bind();

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");
    ImGui::ColorEdit3("Quad Color", m_ClearColor);
    ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, -10.0f, 10.0f);
    ImGui::Text("Press ESC to exit.");
    ImGui::End();
  };

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  float m_RotationSpeed = 1.f;
  float m_Rotation = 0.0f;

  Shader m_Shader;

  std::unique_ptr<VertexBuffer> m_VertexBuffer;
  std::unique_ptr<IndexBuffer> m_IndexBuffer;
  std::unique_ptr<VertexArray> m_VertexArray;

  // GLuint VAO;
};