#pragma once

#include <memory>
#include <string>

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "Logger.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#ifndef ROOT
#define ROOT ""
#endif

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#else
#include <csignal>
#define DEBUG_BREAK() std::raise(SIGTRAP)
#endif

#ifdef _DEBUG
#define ASSERT(x) \
  if (!(x)) DEBUG_BREAK();

#define GLCall(x) \
  GLClearError(); \
  x;              \
  ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define ASSERT(x)
#define GLCall(x) x
#endif

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

inline std::string getRootDir() { return std::string(ROOT); }

inline std::string getFilePath(const std::string& path) { return getRootDir() + path; }

using uint = unsigned int;
using uchar = unsigned char;

using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;

struct RendererConfig {
 public:
  static bool m_Wireframe;
  static bool m_ShowStats;
  static bool m_DepthTest;
  static bool m_CullFace;
  static bool m_Blend;
};

// TODO: Use this method in scenes
struct Renderer {
  // TODO: Use this method in scenes
  static void setup3D() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // TODO: Use this method in scenes
  static void setp2D() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
  }
};