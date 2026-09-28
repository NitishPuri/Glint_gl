#include <windows.h>

#include "Window.h"

void printSysinfo() {
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
  const GLubyte* version = glGetString(GL_VERSION);

  if (!version || !renderer || !vendor || !glslVersion) {
    Logger::error("Failed to get system info");
    return;
  }

  if (version) Logger::log("Version: ", version);
  if (renderer) Logger::log("Renderer: ", renderer);
  if (vendor) Logger::log("Vendor: ", vendor);
  if (glslVersion) Logger::log("GLSL Version: ", glslVersion);

  // listExtensions();

  int count;
  GLFWmonitor** monitors = glfwGetMonitors(&count);
  Logger::log("Number of monitors: ", count);
  for (int i = 0; i < count; ++i) {
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
    Logger::log("Monitor ", i + 1, ": ", mode->width, "x", mode->height, " @ ", mode->refreshRate, "Hz");
  }

  // Print system memory info
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  Logger::log("Total System Memory: ", memInfo.ullTotalPhys / (1024 * 1024), " MB");
  Logger::log("Available System Memory: ", memInfo.ullAvailPhys / (1024 * 1024), " MB");

  // Print CPU info
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  Logger::log("Number of Processors: ", sysInfo.dwNumberOfProcessors);

  // Print threading capabilities
  //   Logger::log("Processor Type: ", sysInfo.dwProcessorType);
  //   Logger::log("Minimum Thread Count: ", sysInfo.dwNumberOfProcessors);
  //   Logger::log("Maximum Thread Count: ", sysInfo.dwNumberOfProcessors);
  //   Logger::log("Page Size: ", sysInfo.dwPageSize, " bytes");
  //   Logger::log("Minimum Application Address: ", sysInfo.lpMinimumApplicationAddress);
  //   Logger::log("Maximum Application Address: ", sysInfo.lpMaximumApplicationAddress);
  //   Logger::log("Active Processor Mask: ", sysInfo.dwActiveProcessorMask);
}

void listExtensions() {
  // Check for extensions
  GLint numExtensions;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
  Logger::log("Number of extensions: ", numExtensions);
  for (int i = 0; i < numExtensions; ++i) {
    const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
    Logger::log("Extension ", i + 1, ": ", extension);
  }
}

#ifdef ENABLE_GL_DEBUG
void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                   const void* userParam) {
  // Ignore non-significant error/warning codes
  // if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
  if (id == 131185 || id == 131218 || id == 131204) return;

  std::cerr << "---------------" << std::endl;
  std::cerr << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      std::cerr << "Source: API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      std::cerr << "Source: Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      std::cerr << "Source: Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      std::cerr << "Source: Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      std::cerr << "Source: Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      std::cerr << "Source: Other";
      break;
  }
  std::cerr << std::endl;

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      std::cerr << "Type: Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      std::cerr << "Type: Deprecated Behaviour";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      std::cerr << "Type: Undefined Behaviour";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      std::cerr << "Type: Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      std::cerr << "Type: Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      std::cerr << "Type: Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      std::cerr << "Type: Push Group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      std::cerr << "Type: Pop Group";
      break;
    case GL_DEBUG_TYPE_OTHER:
      std::cerr << "Type: Other";
      break;
  }
  std::cerr << std::endl;

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      std::cerr << "Severity: high";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      std::cerr << "Severity: medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      std::cerr << "Severity: low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      std::cerr << "Severity: notification";
      break;
  }
  std::cerr << std::endl;
  std::cerr << std::endl;
}
#endif