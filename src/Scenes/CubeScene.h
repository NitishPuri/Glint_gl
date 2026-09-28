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

class CubeScene : public SceneBase {
 public:
  CubeScene() : SceneBase("Cube Scene") {}
  void onAttach() override {
    m_Shader.init(getFilePath("/shaders/simple.vert"), getFilePath("/shaders/simple.frag"));

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cube Data
    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat vertex_buffer_data[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f, 1.0f,  -1.0f, 1.0f};

    // One color for each vertex. They were generated randomly.
    static const GLfloat color_buffer_data[] = {
        0.583f, 0.771f, 0.014f, 0.609f, 0.115f, 0.436f, 0.327f, 0.483f, 0.844f, 0.822f, 0.569f, 0.201f, 0.435f, 0.602f,
        0.223f, 0.310f, 0.747f, 0.185f, 0.597f, 0.770f, 0.761f, 0.559f, 0.436f, 0.730f, 0.359f, 0.583f, 0.152f, 0.483f,
        0.596f, 0.789f, 0.559f, 0.861f, 0.639f, 0.195f, 0.548f, 0.859f, 0.014f, 0.184f, 0.576f, 0.771f, 0.328f, 0.970f,
        0.406f, 0.615f, 0.116f, 0.676f, 0.977f, 0.133f, 0.971f, 0.572f, 0.833f, 0.140f, 0.616f, 0.489f, 0.997f, 0.513f,
        0.064f, 0.945f, 0.719f, 0.592f, 0.543f, 0.021f, 0.978f, 0.279f, 0.317f, 0.505f, 0.167f, 0.620f, 0.077f, 0.347f,
        0.857f, 0.137f, 0.055f, 0.953f, 0.042f, 0.714f, 0.505f, 0.345f, 0.783f, 0.290f, 0.734f, 0.722f, 0.645f, 0.174f,
        0.302f, 0.455f, 0.848f, 0.225f, 0.587f, 0.040f, 0.517f, 0.713f, 0.338f, 0.053f, 0.959f, 0.120f, 0.393f, 0.621f,
        0.362f, 0.673f, 0.211f, 0.457f, 0.820f, 0.883f, 0.371f, 0.982f, 0.099f, 0.879f};

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertex_buffer_data, sizeof(vertex_buffer_data));
    m_ColorBuffer = std::make_unique<VertexBuffer>(color_buffer_data, sizeof(color_buffer_data));
  }

  void onDetach() override {
    m_VertexBuffer.reset();
    // m_IndexBuffer.reset();
    m_VertexArray.reset();
  };

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += m_RotationSpeed * deltaTime * 10;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;
  };

  glm::mat4 getViewMatrix() {
    ImGuiIO &io = ImGui::GetIO();
    float aspect = io.DisplaySize.x / io.DisplaySize.y;

    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View = glm::lookAt(glm::vec3(4, 3, -3),  // Camera is at (4,3,-3), in World Space
                                 glm::vec3(0, 0, 0),   // and looks at the origin
                                 glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP = Projection * View * Model;  // Remember, matrix multiplication is the other way around
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

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    m_ColorBuffer->bind();
    glVertexAttribPointer(1,         // attribute. No particular reason for 1, but must match the layout in the shader.
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    GLCall(glDrawArrays(GL_TRIANGLES, 0, 12 * 3));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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

  std::unique_ptr<VertexBuffer> m_VertexBuffer, m_ColorBuffer;
  std::unique_ptr<VertexArray> m_VertexArray;
};