# Known issues and tech debt

Found while reviewing the code on 2026-09-29. None of these have been fixed in Glint_gl. The intent is
to avoid repeating them when the code is ported into `glint`, rather than to patch this repo.
Severity: 🔴 wrong output or crash risk · 🟠 misleading or fragile · ⚪ cleanup.

## Rendering correctness

| | Where | Issue |
|---|---|---|
| 🔴 | `src/Graphics/FrameBuffer.h:100` + `shaders/shadow_mapping*.frag` | The shadow-map depth texture never sets `GL_TEXTURE_COMPARE_MODE`, yet it is sampled as `sampler2DShadow`. The spec says the result is undefined; some drivers happen to do the comparison anyway. Fix: `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)` (+ `GL_TEXTURE_COMPARE_FUNC GL_LEQUAL`, linear filter for hardware PCF). |
| 🔴 | `src/Scenes/RenderToTexture.h` (`renderToScreen`) | Uniforms `time`, `isDepth`, `depthNear`, `depthFar` and `depthScale` of `wobbly_texture.frag` are never set, so the wobble doesn't animate, the depth view shows raw depth in red, and the depth sliders do nothing. |
| 🟠 | `src/Scenes/UVCubeScene.h`, `BasicShading.h` | `m_Texture` is never bound in `onRender`. It works only because the `Texture` constructor leaves it bound on the *currently active* unit. After `8_shadow_mapping` (which leaves `GL_TEXTURE1` active), the texture ends up on unit 1 while the sampler reads unit 0, and switching textures in scene 3 relies on the same accident. |
| 🟠 | `src/Scenes/BasicShading.h`, `NormalMapping.h:88` | Uniform `V` is given the view-*projection* matrix, which skews the camera-space lighting. |
| 🟠 | `src/Scenes/ShadowMapping.h:153` | At the end of `renderDepth()` there is `glEnableVertexAttribArray(0)` where `glDisableVertexAttribArray(0)` was meant. The depth pass also never binds `m_VertexArray`. |
| 🟠 | `ShadowMapping.h:293`, `RenderToTexture.h:221` | `onWindowResize` only updates the camera aspect; the offscreen FBO keeps its original window size. The shadow map resolution is also tied to the window size. |
| 🟠 | `src/Core/Window.h:94` | The resize callback calls `glViewport` globally, while FBO scenes set their own viewport. This works only because every pass sets it again. |
| 🟠 | global | GL state leaks between scenes (`QuadScene` disables depth test, `ShadowMapping` toggles culling, the UI checkboxes call `glEnable` directly), so the checkbox state can disagree with actual GL state. |

## Robustness

| | Where | Issue |
|---|---|---|
| 🔴 | `src/Graphics/FrameBuffer.h:126` | `m_depthAttachment` is uninitialised. `createDepthAttachment()` calls `cleanupDepthAttachment()` first, which may run `glDelete*` on a garbage name. |
| 🟠 | `src/Graphics/FrameBuffer.h:85` | `cleanupDepthAttachment` deletes the same name as both a renderbuffer and a texture, so it can delete an unrelated object that happens to share the number. |
| 🟠 | `src/Graphics/Shader.cpp:25` | A missing vertex shader file calls `getchar()`, which blocks the app waiting for stdin. A missing fragment shader isn't checked at all. |
| 🟠 | `src/Graphics/Shader.cpp` | Compile and link failures are logged but the program is used anyway. `Shader::init` (line 101) overwrites `m_ID` without deleting the previous program. |
| 🟠 | `src/Graphics/Texture.cpp:26` | The `stbi_load` result isn't checked, so a missing image uploads `nullptr` data silently. |
| 🟠 | all GL wrappers | No copy or move rules (rule of 5); an accidental copy double-deletes the GL name. |
| 🟠 | `src/Graphics/Mesh.cpp` (`computeTangentBasis`) | Assumes a non-indexed triangle list with normals and UVs present; indexes out of bounds otherwise. |

## Logging and debugging

| | Where | Issue |
|---|---|---|
| 🟠 | `src/Core/Renderer.cpp:6` | `Logger::error("OpenGL Error ({0}): {1} {2}:{3}", …)` uses `{}` placeholders with the concatenating `error()`, so the placeholders print literally. Should be `errorf` with `{}`. |
| 🟠 | `src/Graphics/Shader.cpp:69,85` | `Logger::log("%s\n", msg)` uses printf-style formatting with a concatenating logger. The vertex shader log uses `printf` and skips the log file. |
| ⚪ | `src/Core/GuiLayer.h:15` | ImGui GL backend initialised with `"#version 330"` on a 4.6 context (harmless). |
| ⚪ | `ext/glad` | The loader was generated for the **compatibility** profile while the context is core; regenerate as core so removed functions don't show up in autocomplete. |

## Structure

| | Where | Issue |
|---|---|---|
| 🟠 | every scene | Attribute pointers are set every frame instead of being recorded once in the VAO; there is heavy copy-paste between scenes 4–8. |
| ⚪ | `src/Core/SceneManager.h:129` | `unordered_map` makes the scene combo order arbitrary. |
| ⚪ | `src/Graphics/VertexArrayLayout.h:3` | Empty stub; wrong include path (`Renderer.h` is in `Core/`). |
| ⚪ | `src/Graphics/Camera.cpp:41` | Unused `static int cntr`. Camera input reads ImGui IO directly and ignores `WantCaptureMouse`. |
| ⚪ | `src/Core/Renderer.h` | Grab bag: paths, GL debug macros, `using std::…` in a header, config globals, and state presets. |
| ⚪ | `tests/` | Not built; `camera_test.cpp` includes a header that doesn't exist. |
| ⚪ | `src/app.cpp:21` | Window hard-coded to 1800×1600, which is taller than many laptop screens. |

## How glint avoids these

See [`glint/docs/PLAN.md`](../../glint/docs/PLAN.md):
- DSA with one-time VAO setup (`glVertexArrayAttribFormat` / `glVertexArrayVertexBuffer`).
- Shader failures are fatal in Debug and hot-reload keeps the last good program.
- `ImageData`/`MeshData` loaders validate their inputs.
- Wrappers are move-only.
- Shadow sampler compare mode is set on both APIs (in VK it's `compareEnable` on the sampler).
- The render-target size is decoupled from the window.
- The parity tool compares GL and VK output, which catches bugs like the uniform ones above.
