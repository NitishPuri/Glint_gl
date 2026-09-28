#pragma once

// core
#include "Core/SceneBase.h"
#include "Graphics/Camera.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"
#include "Graphics/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class UVCubeScene : public SceneBase {
 public:
  UVCubeScene() : SceneBase("UV Cube Scene"), m_CameraController(getDefaultCameraProps()) {}
  void onAttach() override {
    // m_CameraController.getProps().aspect = (float)width / (float)height;

    m_Shader.init(getFilePath("/shaders/simple_uv.vert"), getFilePath("/shaders/simple_uv.frag"));

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cube Data
    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat vertex_buffer_data[] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,   //
                                                 1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,  //
                                                 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,  //
                                                 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  //
                                                 -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,  //
                                                 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,  //
                                                 -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,   //
                                                 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,  //
                                                 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,   //
                                                 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,  //
                                                 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,   //
                                                 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f};

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();
    m_VertexBuffer = std::make_unique<VertexBuffer>(vertex_buffer_data, sizeof(vertex_buffer_data));
    m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/box.jpg"));
    setBoxUv();
  }

  void setBoxUv() {
    static const GLfloat g_uv_buffer_data_box[] = {
        // Front face
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Back face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Top face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Bottom face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Right face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Left face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    m_UVBuffer = std::make_unique<VertexBuffer>(g_uv_buffer_data_box, sizeof(g_uv_buffer_data_box));
  }

  void setGridUV() {
    static const GLfloat g_uv_buffer_data_box[] = {
        // Front face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Back face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Top face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Bottom face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Right face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        // Left face
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    m_UVBuffer = std::make_unique<VertexBuffer>(g_uv_buffer_data_box, sizeof(g_uv_buffer_data_box));
  }

  void onDetach() override {};

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += m_RotationSpeed * deltaTime * 10;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;

    // Update camera
    m_CameraController.update(deltaTime);
  };

  glm::mat4 getViewMatrix() {
    auto ViewProjection = m_CameraController.getViewProjection();

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));

    glm::mat4 MVP = ViewProjection * Model;  // Remember, matrix multiplication is the other way around
    return MVP;
  }

  void onRender() override {
    // Clear screen
    GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glm::mat4 transform = getViewMatrix();

    // Render Quad
    m_Shader.bind();
    m_Shader.setUniformMat4("MVP", transform);

    m_VertexArray->bind();

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0,         // attribute. No particular reason for 0, but must match the layout in the shader.
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    // 2nd attribute buffer : UV
    glEnableVertexAttribArray(1);
    m_UVBuffer->bind();
    glVertexAttribPointer(1,         // attribute. No particular reason for 1, but must match the layout in the shader.
                          2,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    // GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 12 * 3));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");

    ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, -10.0f, 10.0f);

    std::string tex_name = (m_currentTexture == 0) ? "box.jpg" : "grid.png";

    if (ImGui::BeginCombo("Texture", tex_name.c_str())) {
      if (ImGui::Selectable("box.jpg")) {
        m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/box.jpg"));
        setBoxUv();
      }
      if (ImGui::Selectable("grid.png")) {
        m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/grid.png"));
        setGridUV();
      }
      ImGui::EndCombo();
    }

    // TODO: put this in camera controller
    m_CameraController.onImGuiRender();

    ImGui::End();
  };

  void onWindowResize(int width, int height) override {
    m_CameraController.getProps().aspect = (float)width / (float)height;
    // m_CameraController.recalculate()
  }

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

  float m_RotationSpeed = 1.f;
  float m_Rotation = 0.0f;

  int m_currentTexture = 0;

  Shader m_Shader;
  CameraController m_CameraController;

  std::unique_ptr<VertexArray> m_VertexArray;
  std::unique_ptr<VertexBuffer> m_VertexBuffer;
  std::unique_ptr<VertexBuffer> m_UVBuffer;
  std::unique_ptr<Texture> m_Texture;
};