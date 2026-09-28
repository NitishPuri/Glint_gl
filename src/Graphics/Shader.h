// Shader.h
#pragma once
#include <string>
#include <unordered_map>

#include "Core/Renderer.h"
#include "Texture.h"
#include "glm/glm.hpp"

class Shader {
 public:
  Shader() {}
  Shader(const std::string& vertexPath, const std::string& fragmentPath);
  ~Shader();

  void init(const std::string& vertexPath, const std::string& fragmentPath);

  void bind() const;
  void unbind() const;

  void setUniform1i(const std::string& name, int value);
  void setUniform1f(const std::string& name, float value);

  void setUniform3f(const std::string& name, float x, float y, float z);
  // void setUniform4f(const std::string& name, float x, float y, float z, float w) {}
  void setUniformMat3(const std::string& name, const glm::mat3& matrix);
  void setUniformMat4(const std::string& name, const glm::mat4& matrix);

  GLuint getID() const { return m_ID; }

  void bindTexture(const std::string& uniformName, const unique_ptr<Texture>& texture, GLuint slot = 0) {
    texture->bind(slot);
    setUniform1i(uniformName, slot);
  }

 private:
  GLuint m_ID;
  std::unordered_map<std::string, int> m_UniformLocationCache;
  int getUniformLocation(const std::string& name);
};
