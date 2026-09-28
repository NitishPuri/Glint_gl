// VertexArray.h
#pragma once
#include "Core/Renderer.h"

class VertexArray {
 public:
  VertexArray() {
    GLCall(glGenVertexArrays(1, &m_ID));
    GLCall(glBindVertexArray(m_ID));
  }
  ~VertexArray() { GLCall(glDeleteVertexArrays(1, &m_ID)); }

  void bind() const { GLCall(glBindVertexArray(m_ID)); }
  void unbind() const { GLCall(glBindVertexArray(0)); }

 private:
  unsigned int m_ID;
};
