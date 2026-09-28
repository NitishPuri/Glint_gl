# Module reference

One entry per file in `src/Core` and `src/Graphics`. "GL" lists the OpenGL calls each one wraps.
"Port" says where the piece goes in [`glint`](../../glint/docs/PLAN.md).

## Core

### `app.cpp` — `Application`, `main`
Owns `Window`, `ImGuiLayer` and `SceneManager` **by value**, in that order; declaration order matters,
because `ImGuiLayer` holds a `Window&`. It registers the 8 scenes, loads `8_shadow_mapping` first, and runs
the frame loop (see [architecture.md](architecture.md#frame-loop-applicationrun)). The window size is
hard-coded to 1800×1600.
**Port:** the loop becomes `app_gl/main.cpp`; the registration becomes the technique registry.

### `Core/Window.h` — `Window`
Wraps GLFW. `init()` creates a GL 4.6 core context, with a debug context when `_DEBUG` is defined, loads
glad, turns on vsync, sets up `KHR_debug`, and prints system info. The framebuffer-size callback calls
`glViewport` and forwards to `SceneManager::onWindowResize`. `pollEvents()` also closes the window on ESC.
Everything lives in the header, and it calls `exit()` on failure.
**Port:** `core/window` (API-agnostic; takes a `GraphicsApi`). The `glViewport` call moves into the GL backend.

### `Core/GuiLayer.h` — `ImGuiLayer`
`init()` creates the ImGui context with the GLFW and OpenGL3 backends (GLSL `"#version 330"`).
`beginFrame()` and `endFrame()` wrap `NewFrame` and `Render`/`RenderDrawData`. There is no shutdown.
**Port:** frame logic goes to `core/ui`; backend init moves into each app.

### `Core/SceneBase.h` — `SceneBase`
Abstract scene: `onAttach`, `onDetach`, `onUpdate(dt)`, `onRender()` (pure virtual), `onImGuiRender`,
`onWindowResize(w,h)`, plus a `name`.
**Port:** becomes `gl::Technique`; the VK side gets its own interface with `record(VkCommandBuffer)`.

### `Core/SceneManager.h` — `SceneManager`
A name-to-factory map, plus the current scene as a `shared_ptr`. `loadScene` detaches the old scene,
constructs the new one and attaches it; an empty name means none. When no scene is loaded, `onRender`
clears to grey. `onImGuiRender` draws the "Scene Control Panel" with a scene combo, the global render
toggles (wireframe, depth test, blend) and stats (FPS, and ImGui's own vertex/index counts, which are not
the scene's).
**Port:** a `Registry<T>` template; the render toggles become per-backend.

### `Core/Renderer.h` / `Renderer.cpp`
A collection of utilities, despite the name:
- `ROOT`, `getRootDir()`, `getFilePath(path)` for resource paths.
- `GLCall(x)` / `ASSERT(x)` / `DEBUG_BREAK()` (Debug only), `GLClearError`, `GLLogCall`.
- Aliases: `uint`, `uchar`, `using std::unique_ptr/make_unique/shared_ptr`.
- `RendererConfig`: static bools (`m_Wireframe`, `m_ShowStats`, `m_DepthTest`, `m_CullFace`, `m_Blend`).
- `Renderer::setup3D()` / `setp2D()` (sic): state presets.
**Port:** split into `core/paths`, `gl/debug`, and per-backend config.

### `Core/Logger.h` / `Logger.cpp` — `Logger`
Static logger writing to the console and `log.txt` (append mode). `log`/`error` concatenate their
arguments with `<<`; `logf`/`errorf` use `{}` formatting through `fmtlib` (an alias for `std` or `fmt`,
see [build.md](build.md)). Includes `operator<<` for glm types and a `formatter<ImVec2>`.
**Port:** `core/log` on top of fmt.

### `Core/ScopedTimer.h` — `ScopedTimer`
RAII CPU timer that logs "`<name>` took X ms" on destruction. Used around mesh indexing.
**Port:** `core/timer`.

### `Core/VBOIndex.h/.cpp` — `indexVBO`, `indexVBO_TBN`
Taken from opengl-tutorial's `vboindexer`. It turns a non-indexed triangle list into a deduplicated vertex
set plus indices.
- `indexVBO` uses a `std::map<PackedVertex,index_t>`, so matches must be exact.
- `indexVBO_TBN` does a **linear search** with a 0.01 epsilon, which is O(n²), and averages the tangents
  and bitangents of merged vertices.

Suzanne goes from 2904 to 590 vertices; the cylinder from 192 to 66.
**Port:** `core/assets` (pure CPU; a good first unit-test target).

### `Core/Sysinfo.cpp` — `printSysinfo`, `listExtensions`, `glDebugOutput`
Logs the GL version, renderer, vendor and GLSL version, the monitors and their modes, and system RAM and
CPU count (`sysinfo()` on Linux, Win32 APIs on Windows). Also contains the `KHR_debug` callback, which
ignores IDs 131185, 131218 and 131204 (NVIDIA noise).
**Port:** `gl/debug` + `core/sysinfo`.

## Graphics

### `Graphics/Shader.h/.cpp` — `Shader`
`init(vertPath, fragPath)` reads, compiles and links through the free function `LoadShaders`, which comes
from opengl-tutorial. Uniform locations are cached in an `unordered_map`. There are setters for
`1i, 1f, 3f, mat3, mat4`, and `bindTexture(name, tex, slot)` does `glActiveTexture` + bind + `glUniform1i`.
Compile and link errors are logged but not treated as failures.
GL: `glCreateShader/ShaderSource/CompileShader/CreateProgram/AttachShader/LinkProgram/UseProgram/Uniform*`.
**Port:** `gl/shader` with hot reload and failure handling. On the VK side the equivalent is SPIR-V
modules + pipeline + descriptor layout.

### `Graphics/VertexBuffer.h` — `VertexBuffer`
`glGenBuffers` + `glBufferData(GL_ARRAY_BUFFER, …, GL_STATIC_DRAW)` in the constructor; `bind`/`unbind`.

### `Graphics/IndexBuffer.h` — `IndexBuffer`
Same for `GL_ELEMENT_ARRAY_BUFFER`, with `uint32` indices; stores `count`.

### `Graphics/VertexArray.h` — `VertexArray`
`glGenVertexArrays` + bind in the constructor. It never records a layout, because scenes set attributes
every frame.

### `Graphics/VertexArrayLayout.h` — `VertexArrayLayout`
An empty stub (a TODO). Its `#include "Renderer.h"` would fail to compile if the file were included;
nothing includes it.

### `Graphics/Texture.h/.cpp` — `Texture`
Two constructors:
- `Texture(w,h)`: empty RGBA8 texture, linear filtering, clamp-to-edge. Used as an FBO colour attachment.
- `Texture(path)`: stb_image load, flipped vertically, forced to 4 channels; RGBA8, mipmaps, linear
  filtering, repeat wrapping.

`bind(slot)` = `glActiveTexture(GL_TEXTURE0+slot)` + `glBindTexture`.
**Port:** `core/assets::loadImage` (CPU) + `gl/texture` (DSA) + `vk/image` (staging, layout transitions,
mip blits).

### `Graphics/FrameBuffer.h` — `FrameBuffer`
`create(w,h)` generates **and binds** the FBO. After that:
- `createColorAttachment()` adds an RGBA8 `Texture`.
- `createDepthAttachment(None|RenderBuffer|Texture)` adds either a `GL_DEPTH_COMPONENT` renderbuffer or a
  `GL_DEPTH_COMPONENT24` texture with nearest filtering and clamp-to-edge.

Each create step checks framebuffer completeness and throws if it fails. `getDepthRenderBuffer()` returns
the depth *texture or renderbuffer* name. Draw buffers are set by the scenes, not here.
**Port:** `gl/framebuffer`; on the VK side, an offscreen image + view + dynamic rendering attachment.

### `Graphics/Mesh.h/.cpp` — `Mesh`
Loads an OBJ with tinyobjloader into **flat (non-indexed) per-vertex arrays**: positions, normals and UVs,
with `indices = 0..n-1`. `index()` deduplicates through `indexVBO`. `indexWithTangentBasis()` runs
`computeTangentBasis()` (per-triangle T/B, Gram-Schmidt against N, handedness fix-up) followed by
`indexVBO_TBN`. The `.mtl` files are ignored.
**Port:** `core/assets::MeshData` (+ `makeCube/makeQuad`), shared unchanged by both backends.

### `Graphics/Camera.h/.cpp` — `CameraProps`, `CameraController`
- `CameraProps` holds fov, aspect, near, far, position, target and up. `getDefaultCameraProps()` returns
  45°, 4:3, 0.1–100, eye at (4,3,-3), looking at the origin.
- `CameraController::update(dt)` reads ImGui's IO: aspect from `DisplaySize` and right-drag for movement.
  It has 4 modes:
  - `NONE`
  - `FREE`: translate along forward and right
  - `ORBIT`: spherical coordinates around the target
  - `ARCBALL` (default): delegates to `arcball_camera_update`, using middle and right mouse buttons and the wheel
- `getViewProjection()` is cached; `getProjectionMatrix()` and `getViewMatrix()` are computed on demand.
- `onImGuiRender()` draws the "Camera Control Panel" (mode, speed, FOV, near, far, position readout).

Projections come from `glm::perspective` with GL clip conventions.
**Port:** `core/camera` with a `ClipDepth` flag and input decoupled from ImGui (the VK projection needs
depth `[0,1]` plus a Y flip).
