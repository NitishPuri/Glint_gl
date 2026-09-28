#pragma once

#include <memory>

// core
#include "Core/SceneBase.h"
#include "Core/ScopedTimer.h"
#include "Core/VBOIndex.h"
#include "Graphics/Camera.h"
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

class NormalMapping : public SceneBase {
 public:
  NormalMapping() : SceneBase("Normal Mapping"), m_CameraController(getDefaultCameraProps()) {
    m_CameraController.getProps().position = glm::vec3(0, 0, 5);
  }
  void onAttach() override {
    // m_Shader.init(getFilePath("/shaders/standard_shading.vert"), getFilePath("/shaders/standard_shading.frag"));
    m_Shader.init(getFilePath("/shaders/normal_mapping.vert"), getFilePath("/shaders/normal_mapping.frag"));

    // TODO: Move to something like Renderer::setup3D() ?
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/cylinder/cylinder.obj"));
    m_Mesh->indexWithTangentBasis();

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();

    const auto &vertices = m_Mesh->getVertices();
    const auto &indices = m_Mesh->getIndices();
    const auto &tex_coords = m_Mesh->getTexCoords();
    const auto &normals = m_Mesh->getNormals();
    const auto &tangents = m_Mesh->getTangents();
    const auto &bitangents = m_Mesh->getBitangents();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(glm::vec3));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), uint(indices.size()));
    m_NormalBuffer = std::make_unique<VertexBuffer>(normals.data(), normals.size() * sizeof(glm::vec3));
    m_UVBuffer = std::make_unique<VertexBuffer>(tex_coords.data(), tex_coords.size() * sizeof(glm::vec2));
    m_TangentBuffer = std::make_unique<VertexBuffer>(tangents.data(), tangents.size() * sizeof(glm::vec3));
    m_BitangentBuffer = std::make_unique<VertexBuffer>(bitangents.data(), bitangents.size() * sizeof(glm::vec3));

    m_DiffuseTexture = std::make_unique<Texture>(getFilePath("/res/cylinder/diffuse.jpg"));
    m_NormalTexture = make_unique<Texture>(getFilePath("/res/cylinder/normal.jpg"));
    m_SpecularTexture = make_unique<Texture>(getFilePath("/res/cylinder/specular.jpg"));
  }

  void onDetach() override {};

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    // m_Rotation += m_RotationSpeed * deltaTime * 10;
    // if (m_Rotation > 360.0f) m_Rotation -= 360.0f;

    // Update camera
    m_CameraController.update(deltaTime);
  };

  void onRender() override {
    GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glm::mat4 ProjectionMatrix = m_CameraController.getProjectionMatrix();
    glm::mat4 ViewMatrix = m_CameraController.getViewProjection();
    glm::mat4 ViewProjection = m_CameraController.getViewProjection();
    glm::mat4 ModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
    glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
    glm::mat4 MVP = ViewProjection * ModelMatrix;

    // Uniforms
    m_Shader.bind();
    {
      // Camera
      m_Shader.setUniformMat4("MVP", MVP);
      m_Shader.setUniformMat4("V", ViewProjection);
      m_Shader.setUniformMat4("M", ModelMatrix);
      m_Shader.setUniformMat3("MV3x3", ModelView3x3Matrix);

      // Lights
      m_Shader.setUniform3f("LightPosition_worldspace", m_LightPos.x, m_LightPos.y, m_LightPos.z);
      m_Shader.setUniform3f("LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
      m_Shader.setUniform1f("LightPower", m_LightPower);

      // Material
      m_Shader.setUniform1f("MaterialAmbient", m_AmbientStrength);
      m_Shader.setUniform1f("MaterialSpecular", m_SpecularStrength);
    }

    m_Shader.bindTexture("DiffuseTextureSampler", m_DiffuseTexture, 0);
    m_Shader.bindTexture("NormalTextureSampler", m_NormalTexture, 1);
    m_Shader.bindTexture("SpecularTextureSampler", m_SpecularTexture, 2);

    m_VertexArray->bind();

    // vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    //  UV
    glEnableVertexAttribArray(1);
    m_UVBuffer->bind();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // normals
    glEnableVertexAttribArray(2);
    m_NormalBuffer->bind();
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // tangents
    glEnableVertexAttribArray(3);
    m_TangentBuffer->bind();
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // bitangents
    glEnableVertexAttribArray(4);
    m_BitangentBuffer->bind();
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // index buffer
    m_IndexBuffer->bind();

    auto index_count = m_Mesh->getIndices().size();

    GLCall(glDrawElements(GL_TRIANGLES, int(index_count), GL_UNSIGNED_INT, nullptr));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");

    ImGui::SliderFloat3("Light Position", glm::value_ptr(m_LightPos), -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_LightColor));
    ImGui::SliderFloat("Light Power", &m_LightPower, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Material Ambient", &m_AmbientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_SpecularStrength, 0.0f, 10.0f);

    // TODO: put this in camera controller
    m_CameraController.onImGuiRender();

    ImGui::End();
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

  Shader m_Shader;
  CameraController m_CameraController;

  std::unique_ptr<Mesh> m_Mesh;
  std::unique_ptr<Texture> m_DiffuseTexture;
  std::unique_ptr<Texture> m_NormalTexture, m_SpecularTexture;

  std::unique_ptr<VertexArray> m_VertexArray;
  std::unique_ptr<VertexBuffer> m_VertexBuffer, m_NormalBuffer, m_UVBuffer;
  std::unique_ptr<VertexBuffer> m_TangentBuffer, m_BitangentBuffer;
  std::unique_ptr<IndexBuffer> m_IndexBuffer;
};