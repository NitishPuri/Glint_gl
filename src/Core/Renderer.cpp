#include "Renderer.h"

void GLClearError() { while (glGetError() != GL_NO_ERROR); }
bool GLLogCall(const char* function, const char* file, int line) {
  while (GLenum error = glGetError()) {
    Logger::error("OpenGL Error ({0}): {1} {2}:{3}", error, function, file, line);
    return false;
  }
  return true;
}

bool RendererConfig::m_Wireframe = false;
bool RendererConfig::m_ShowStats = false;
bool RendererConfig::m_DepthTest = true;
bool RendererConfig::m_CullFace = true;
bool RendererConfig::m_Blend = false;