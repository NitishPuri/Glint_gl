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

// Reference:
//   https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
class ShadowMapping : public SceneBase {
 public:
  ShadowMapping() : SceneBase("Shadow Mapping"), m_CameraController(getDefaultCameraProps()) {
    m_CameraController.getProps().position = glm::vec3(5, 4, -13);
  }
  void onAttach() override {
    Logger::log("ShadowMapping::onAttach()\n\n");

    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);

    m_depthShader.init(getFilePath("/shaders/depth_rtt.vert"), getFilePath("/shaders/depth_rtt.frag"));
    m_shadowMapping.init(getFilePath("/shaders/shadow_mapping.vert"), getFilePath("/shaders/shadow_mapping.frag"));
    m_shadowMapping_simple.init(getFilePath("/shaders/shadow_mapping_simple.vert"),
                                getFilePath("/shaders/shadow_mapping_simple.frag"));

    Renderer::setup3D();

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/room/room_thickwalls.obj"));
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

    m_Texture = std::make_unique<Texture>(getFilePath("/res/room/uvmap.jpg"));

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
    // m_offscreenBuffer.createColorAttachment();
    // and the depth buffer
    m_offscreenBuffer.createDepthAttachment(FrameBuffer::DepthAttachmentType::Texture);

    // No color output in the bound framebuffer, only depth.
    // TODO: Move this into FrameBuffer class?
    // check if this is needed
    glDrawBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //   Logger::error("Framebuffer not complete!");
    //   return;
    //   // return false
    // }

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };

    m_quadVertexBuffer = make_unique<VertexBuffer>(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data));

    m_RTTInitialized = true;
  }

  void onDetach() override { glDisable(GL_CULL_FACE); };

  void onUpdate(float deltaTime) override {
    // Update camera
    m_CameraController.update(deltaTime);
  };

  void renderDepth() {
    m_offscreenBuffer.bind();
    glViewport(0, 0, int(ImGui::GetIO().DisplaySize.x), int(ImGui::GetIO().DisplaySize.y));

    // We don't use bias in the shader, but instead we draw back faces,
    // which are already separated from the front faces by a small distance
    // (if your geometry is made this way)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Cull back-facing triangles -> draw only front-facing triangles

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    m_depthShader.bind();

    glm::vec3 lightInvDir = m_lightDirIsNeg ? -m_LightPos : m_LightPos;

    glm::mat4 depthProjectionMatrix, depthViewMatrix;
    // Compute the MVP matrix from the light's point of view
    if (m_SpotLight) {
      depthProjectionMatrix = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
      depthViewMatrix = glm::lookAt(m_LightPos, m_LightPos - lightInvDir, glm::vec3(0, 1, 0));
    } else {
      depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
      depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }

    glm::mat4 depthModelMatrix = glm::mat4(1.0);
    m_depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    m_depthShader.setUniformMat4("depthMVP", m_depthMVP);

    // m_VertexArray->bind();

    //  1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    m_IndexBuffer->bind();

    GLCall(glDrawElements(GL_TRIANGLES, int(m_Mesh->getIndices().size()), GL_UNSIGNED_INT, nullptr));

    glEnableVertexAttribArray(0);
  }

  void renderToScreen() {
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_offscreenBuffer.unbind();

    auto &shader = m_useSimpleShadowMapping ? m_shadowMapping_simple : m_shadowMapping;

    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
    auto windowHeight = int(ImGui::GetIO().DisplaySize.y);
    glViewport(0, 0, windowWidth, windowHeight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Cull back-facing triangles -> draw only front-facing triangles

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.bind();

    glm::mat4 Projection = m_CameraController.getProjectionMatrix();
    glm::mat4 View = m_CameraController.getViewMatrix();
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 MVP = Projection * View * Model;

    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,  //
                         0.0, 0.5, 0.0, 0.0,  //
                         0.0, 0.0, 0.5, 0.0,  //
                         0.5, 0.5, 0.5, 1.0);
    glm::mat4 depthBiasMVP = biasMatrix * m_depthMVP;

    // Camera
    shader.setUniformMat4("MVP", MVP);
    shader.setUniformMat4("V", View);
    shader.setUniformMat4("M", Model);
    shader.setUniformMat4("DepthBiasMVP", depthBiasMVP);

    glm::vec3 lightInvDir = m_lightDirIsNeg ? -m_LightPos : m_LightPos;

    shader.setUniform3f("LightInvDirection_worldspace", lightInvDir.x, lightInvDir.y, lightInvDir.z);

    // Bind our texture in Texture Unit 0
    shader.bindTexture("myTextureSampler", m_Texture, 0);

    //   shader.bindTexture("shadowMap", m_offscreenBuffer.getDepthRenderBuffer(), 1);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_offscreenBuffer.getDepthRenderBuffer());
    shader.setUniform1i("shadowMap", 1);

    // Lights
    shader.setUniform3f("LightPosition_worldspace", m_LightPos.x, m_LightPos.y, m_LightPos.z);
    shader.setUniform3f("LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
    shader.setUniform1f("LightPower", m_LightPower);

    // Material
    shader.setUniform1f("MaterialAmbient", m_AmbientStrength);
    shader.setUniform1f("MaterialSpecular", m_SpecularStrength);

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
  }

  void onRender() override {
    if (!m_RTTInitialized) {
      Logger::error("RTT not initialized yet. Retrying...");
      setupRtt();
      return;
    }
    renderDepth();
    renderToScreen();
  }

  ImVec2 calculatePanelSize(int imageWidth, int imageHeight) {
    ImVec2 available_size = ImGui::GetContentRegionAvail();
    float aspect_ratio = static_cast<float>(imageWidth) / imageHeight;
    ImVec2 image_size;

    if (available_size.x / aspect_ratio <= available_size.y) {
      image_size.x = available_size.x;
      image_size.y = available_size.x / aspect_ratio;
    } else {
      image_size.x = available_size.y * aspect_ratio;
      image_size.y = available_size.y;
    }

    return image_size;
  }

  void onImGuiRender() override {
    ImGui::Begin("RTT Panel");

    ImGui::SliderFloat3("Light Position", glm::value_ptr(m_LightPos), -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_LightColor));
    ImGui::SliderFloat("Light Power", &m_LightPower, 0.0f, 5.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Material Ambient", &m_AmbientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_SpecularStrength, 0.0f, 10.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show Depth Buffer", &m_ShowDepthBuffer);
    ImGui::Checkbox("LightDir is -", &m_lightDirIsNeg);
    ImGui::Checkbox("Spotlight ? ", &m_SpotLight);
    ImGui::Checkbox("Simple ? ", &m_useSimpleShadowMapping);

    if (m_ShowDepthBuffer) {
      auto imageSize = calculatePanelSize(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
      ImGui::Image(m_offscreenBuffer.getDepthRenderBuffer(), imageSize, {0, 1}, {1, 0});
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
  float m_LightPower = 2.f;
  bool m_SpotLight = false;

  float m_AmbientStrength = 0.1f;
  float m_SpecularStrength = 0.3f;

  // used to visualize depth
  bool m_ShowDepthBuffer = false;
  float m_depthNear = 0.1f;
  float m_depthFar = 100.0f;
  float m_depthScale = 1.0f;

  CameraController m_CameraController;

  Shader m_depthShader;
  unique_ptr<Mesh> m_Mesh;
  unique_ptr<VertexArray> m_VertexArray;
  unique_ptr<VertexBuffer> m_VertexBuffer, m_NormalBuffer, m_UVBuffer;
  unique_ptr<IndexBuffer> m_IndexBuffer;
  unique_ptr<Texture> m_Texture;

  // TODO: create abstraction for Framebuffer, or the genral concept of RTT?
  bool m_RTTInitialized = false;
  FrameBuffer m_offscreenBuffer;
  unique_ptr<VertexBuffer> m_quadVertexBuffer;

  Shader m_shadowMapping;
  Shader m_shadowMapping_simple;
  glm::mat4 m_depthMVP;
  bool m_useSimpleShadowMapping = false;

  // TODO: This is for debugging,
  //  still not sure if the light direction is correct
  //  there are other problems with the shadow as well,
  //  like i think the sphere is not receiving any shadows.
  //  need closer inspection and debugging
  bool m_lightDirIsNeg = false;
};