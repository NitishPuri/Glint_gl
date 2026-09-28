#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

// #include "Logger.h"
#include "glm/gtc/type_ptr.hpp"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << VertexShaderStream.rdbuf();
    VertexShaderCode = sstr.str();
    VertexShaderStream.close();
  } else {
    Logger::error("Cant open vertex shader source, ", vertex_file_path);
    getchar();
    return 0;
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << FragmentShaderStream.rdbuf();
    FragmentShaderCode = sstr.str();
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  Logger::log("Compiling shader : ", vertex_file_path);
  char const* VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }

  // Compile Fragment Shader
  Logger::log("Compiling shader : ", fragment_file_path);
  char const* FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    Logger::log("%s\n", &FragmentShaderErrorMessage[0]);
  }

  // Link the program
  Logger::log("Linking program");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    Logger::log("%s\n", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) : m_ID(0) {
  m_ID = LoadShaders(vertexPath.c_str(), fragmentPath.c_str());
}

void Shader::init(const std::string& vertexPath, const std::string& fragmentPath) {
  m_ID = LoadShaders(vertexPath.c_str(), fragmentPath.c_str());
}

Shader::~Shader() { glDeleteProgram(m_ID); }

void Shader::bind() const { GLCall(glUseProgram(m_ID)); }

void Shader::unbind() const { GLCall(glUseProgram(0)); }

void Shader::setUniform1i(const std::string& name, int value) { GLCall(glUniform1i(getUniformLocation(name), value)); }

void Shader::setUniform1f(const std::string& name, float value) { glUniform1f(getUniformLocation(name), value); }

void Shader::setUniform3f(const std::string& name, float x, float y, float z) {
  GLCall(glUniform3f(getUniformLocation(name), x, y, z));
}

void Shader::setUniformMat3(const std::string& name, const glm::mat3& matrix) {
  glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& matrix) {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

int Shader::getUniformLocation(const std::string& name) {
  if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) return m_UniformLocationCache[name];

  int location = glGetUniformLocation(m_ID, name.c_str());
  if (location == -1) Logger::error("Warning: uniform '", name, "' doesn't exist!");

  m_UniformLocationCache[name] = location;
  return location;
}