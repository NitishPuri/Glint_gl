#pragma once

// core
#include "Core/SceneBase.h"
#include "Graphics/Camera.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"
#include "Graphics/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class RenderToTexture : public SceneBase {
 public:
  RenderToTexture() : SceneBase("Render To Texture"), m_CameraController(getDefaultCameraProps()) {
    m_CameraController.getProps().position = glm::vec3(0, 0, 5);
  }
  void onAttach() override {
    Logger::log("RenderToTexture::onAttach()\n\n");
    m_standarShader.init(getFilePath("/shaders/standard_shading.vert"), getFilePath("/shaders/standard_shading.frag"));

    Renderer::setup3D();

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/suzanne.obj"));
    m_Mesh->index();

    m_VertexArray = std::make_unique<VertexArray>();

    const auto &vertices = m_Mesh->getVertices();
    const auto &indices = m_Mesh->getIndices();
    const auto &tex_coords = m_Mesh->getTexCoords();
    const auto &normals = m_Mesh->getNormals();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(glm::vec3));
    m_NormalBuffer = std::make_unique<VertexBuffer>(normals.data(), normals.size() * sizeof(glm::vec3));
    m_UVBuffer = std::make_unique<VertexBuffer>(tex_coords.data(), tex_coords.size() * sizeof(glm::vec2));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), int(indices.size()));

    m_Texture = std::make_unique<Texture>(getFilePath("/res/suzanne.jpg"));

    /// Setup RTT
    setupRtt();
  }

  void setupRtt() {
    if (m_RTTInitialized) return;

    auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
    auto windowHeight = int(ImGui::GetIO().DisplaySize.y);
    Logger::log("Window Width: ", windowWidth, " Window Height: ", windowHeight);
    if (windowWidth < 0 || windowHeight < 0) {
      Logger::error("Window width or height is not set yet. Will try again later.");
      return;
    }

    m_offscreenBuffer.create(windowWidth, windowHeight);
    // generate and bind texture we are going to render to
    // this now also sets the texture as the currentcolor attachment #0
    m_offscreenBuffer.createColorAttachment();
    // and the depth buffer
    m_offscreenBuffer.createDepthAttachment(FrameBuffer::DepthAttachmentType::Texture);

    // TODO: Move this into FrameBuffer class?
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);  // "1" is the size of DrawBuffers

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };

    m_quadVertexBuffer = make_unique<VertexBuffer>(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data));
    // Create and compile our GLSL program from the shaders
    m_quadShader.init(getFilePath("/shaders/passthrough.vert"), getFilePath("/shaders/wobbly_texture.frag"));

    m_RTTInitialized = true;
  }

  void onDetach() override { glDisable(GL_CULL_FACE); };

  void onUpdate(float deltaTime) override {
    // Update camera
    m_CameraController.update(deltaTime);
  };

  void renderToTexture() {
    m_offscreenBuffer.bind();
    glViewport(0, 0, int(ImGui::GetIO().DisplaySize.x), int(ImGui::GetIO().DisplaySize.y));

    // Clear the screen
    GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Use our shader
    m_standarShader.bind();

    glm::mat4 View = m_CameraController.getViewProjection();
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 MVP = View * Model;

    // Uniforms
    m_standarShader.bind();
    {
      // Camera
      m_standarShader.setUniformMat4("MVP", MVP);
      m_standarShader.setUniformMat4("V", View);
      m_standarShader.setUniformMat4("M", Model);

      // Lights
      m_standarShader.setUniform3f("LightPosition_worldspace", m_LightPos.x, m_LightPos.y, m_LightPos.z);
      m_standarShader.setUniform3f("LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
      m_standarShader.setUniform1f("LightPower", m_LightPower);

      // Material
      m_standarShader.setUniform1f("MaterialAmbient", m_AmbientStrength);
      m_standarShader.setUniform1f("MaterialSpecular", m_SpecularStrength);
    }

    m_standarShader.bindTexture("diffuseSampler", m_Texture, 0);

    m_VertexArray->bind();

    // TODO: Refactor these calls into the mesh or into a mesh renderer class maybe?
    //  1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // 2nd attribute buffer : UV
    glEnableVertexAttribArray(1);
    m_UVBuffer->bind();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    m_NormalBuffer->bind();
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    m_IndexBuffer->bind();

    auto indices_count = int(m_Mesh->getIndices().size());
    GLCall(glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, nullptr));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    m_offscreenBuffer.unbind();
  }

  void renderToScreen() {
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0, 0, int(ImGui::GetIO().DisplaySize.x), int(ImGui::GetIO().DisplaySize.y));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use passthrough quad shader
    m_quadShader.bind();

    // Bind our texture in Texture Unit 0
    if (m_ShowDepthBuffer) {
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, m_offscreenBuffer.getDepthRenderBuffer());
    } else {
      m_quadShader.bindTexture("renderedTexture", m_offscreenBuffer.getColorAttachment(), 0);
    }

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_quadVertexBuffer->bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, 6);  // 2*3 indices starting at 0 -> 2 triangles

    glDisableVertexAttribArray(0);
  }

  void onRender() override {
    if (!m_RTTInitialized) {
      Logger::error("RTT not initialized yet. Retrying...");
      setupRtt();
      return;
    }
    renderToTexture();
    renderToScreen();
  }

  void onImGuiRender() override {
    ImGui::Begin("RTT Panel");

    ImGui::SliderFloat3("Light Position", glm::value_ptr(m_LightPos), -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_LightColor));
    ImGui::SliderFloat("Light Power", &m_LightPower, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Material Ambient", &m_AmbientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_SpecularStrength, 0.0f, 10.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show Depth Buffer", &m_ShowDepthBuffer);

    if (m_ShowDepthBuffer) {
      ImGui::SliderFloat("Depth Near", &m_depthNear, 0.001f, 0.1f);
      ImGui::SliderFloat("Depth Far", &m_depthFar, 0.1f, 10.0f);
      ImGui::SliderFloat("Depth Scale", &m_depthScale, 0.1f, 100.0f);
    }

    ImGui::End();

    m_CameraController.onImGuiRender();
  };

  void onWindowResize(int width, int height) override {
    m_CameraController.getProps().aspect = (float)width / (float)height;
  }

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

  float m_RotationSpeed = 1.f;
  float m_Rotation = 0.0f;

  glm::vec3 m_LightPos = glm::vec3(4, 4, 4);
  glm::vec3 m_LightColor = glm::vec3(1, 1, 1);
  float m_LightPower = 50.f;

  float m_AmbientStrength = 0.1f;
  float m_SpecularStrength = 0.3f;

  // used to visualize depth
  bool m_ShowDepthBuffer = false;
  float m_depthNear = 0.1f;
  float m_depthFar = 100.0f;
  float m_depthScale = 1.0f;

  CameraController m_CameraController;

  Shader m_standarShader;
  unique_ptr<Mesh> m_Mesh;
  unique_ptr<VertexArray> m_VertexArray;
  unique_ptr<VertexBuffer> m_VertexBuffer, m_NormalBuffer, m_UVBuffer;
  unique_ptr<IndexBuffer> m_IndexBuffer;
  unique_ptr<Texture> m_Texture;

  // TODO: create abstraction for the genral concept of RTT? RenderToTarget class?
  bool m_RTTInitialized = false;
  Shader m_quadShader;
  FrameBuffer m_offscreenBuffer;
  unique_ptr<VertexBuffer> m_quadVertexBuffer;
};
