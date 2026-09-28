#include "Texture.h"

#include "Core/Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::Texture(int width, int height) : m_ID(0), m_Width(width), m_Height(height), m_BPP(0), m_FilePath("") {
  GLCall(glGenTextures(1, &m_ID));
  GLCall(glBindTexture(GL_TEXTURE_2D, m_ID));
  GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

  // default filter and wrap,
  // TODO: not sure if these are needed here, should these be set only before rendering? and have no real affect when
  // being set here? or do these modes get set on the current active texture and remain set until changed, per texture?
  // also, not sure what the best defaults should be.
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

Texture::Texture(const std::string& path) : m_ID(0), m_Width(0), m_Height(0), m_BPP(0), m_FilePath(path) {
  Logger::log("Loading texture: ", path);
  stbi_set_flip_vertically_on_load(1);
  unsigned char* local_buffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

  GLCall(glGenTextures(1, &m_ID));
  GLCall(glBindTexture(GL_TEXTURE_2D, m_ID));

  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

  // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer));
  GLCall(glGenerateMipmap(GL_TEXTURE_2D));

  if (local_buffer) {
    stbi_image_free(local_buffer);
  }
}

Texture::~Texture() { GLCall(glDeleteTextures(1, &m_ID)); }

void Texture::bind(unsigned int slot) const {
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  GLCall(glBindTexture(GL_TEXTURE_2D, m_ID));
}

void Texture::unbind() const { GLCall(glBindTexture(GL_TEXTURE_2D, 0)); }
