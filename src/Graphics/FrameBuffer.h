
#pragma once

#include "Core/Renderer.h"
#include "Texture.h"

/*  TODO:
Add buider pattern for better interfaces?
FrameBuffer& withColorAttachment();
FrameBuffer& withDepthAttachment(DepthAttachmentType type);

// Add state validation
bool isComplete() const;
bool hasDepthAttachment() const;
*/

class FrameBuffer {
 public:
  enum class DepthAttachmentType {
    None,
    RenderBuffer,  // faster, cant sample from it
    Texture        // slower, can sample from it
  };

  ~FrameBuffer() {
    cleanup();
    // glDeleteFramebuffers(1, &m_fbo);
    // glDeleteRenderbuffers(1, &m_depthRenderBuffer);
  }

  void create(int width, int height) {
    m_Width = width;
    m_Height = height;
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  }

  void createColorAttachment() {
    m_ColorAttachment = make_unique<Texture>(m_Width, m_Height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorAttachment->getID(), 0);

    setTextureParameters();

    checkFramebufferStatus();
  }

  void createDepthAttachment(DepthAttachmentType type) {
    cleanupDepthAttachment();

    switch (type) {
      case DepthAttachmentType::RenderBuffer:
        createDepthRenderBuffer();
        break;
      case DepthAttachmentType::Texture:
        createDepthTexture();
        break;
      default:
        break;
    }

    checkFramebufferStatus();
  }

  void bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }
  void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  const unique_ptr<Texture>& getColorAttachment() { return m_ColorAttachment; }
  // Texture* getDepthAttachment() { return m_DepthAttachment.get(); }
  GLuint getID() const { return m_fbo; }
  GLuint getDepthRenderBuffer() const { return m_depthAttachment; }

 private:
  void cleanup() {
    if (m_fbo != 0) {
      glDeleteFramebuffers(1, &m_fbo);
      m_fbo = 0;
    }
    cleanupDepthAttachment();
    m_ColorAttachment.reset();
  }

  void cleanupDepthAttachment() {
    if (m_depthAttachment != 0) {
      glDeleteRenderbuffers(1, &m_depthAttachment);
      glDeleteTextures(1, &m_depthAttachment);
      m_depthAttachment = 0;
    }
  }

  void createDepthRenderBuffer() {
    glGenRenderbuffers(1, &m_depthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);
  }

  void createDepthTexture() {
    glGenTextures(1, &m_depthAttachment);
    glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    setTextureParameters();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthAttachment, 0);
  }

  void setTextureParameters() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  void checkFramebufferStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      throw std::runtime_error("Framebuffer is not complete!");
    }
  }

  GLuint m_fbo = 0;

  // TODO: multiple render targets
  std::vector<std::unique_ptr<Texture>> m_colorAttachments;
  unique_ptr<Texture> m_ColorAttachment;  

  int m_Width = -1, m_Height = -1;
  GLuint m_depthAttachment;
};