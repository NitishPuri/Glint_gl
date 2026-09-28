# Scenes

Registered in `app.cpp` in this order. Each scene follows the matching opengl-tutorial.org chapter.
Every scene creates a VAO but **sets attribute pointers every frame**, with non-interleaved buffers (see
the attribute convention in [architecture.md](architecture.md#vertex-attribute-location-convention)).
The "VK port" line for each scene points out what will be new when this technique is written in Vulkan,
in Phases 3 and 5 of the glint plan.

| # | Registry name | Class (`src/Scenes/`) | Technique | Passes | Assets |
|---|---|---|---|---|---|
| 1 | `1_quad` | `QuadScene.h` | Indexed 2D quad, per-vertex colour, rotation uniform | 1 | — |
| 2 | `2_cube` | `CubeScene.h` | 3D cube, MVP, depth test, per-vertex colours | 1 | — |
| 3 | `3_cube_uv` | `UVCubeScene.h` | Textured cube, camera controller, texture switching | 1 | `res/textures/box.jpg`, `grid.png` |
| 4 | `4_basic_shading` | `BasicShading.h` | OBJ loading + Blinn-style Phong lighting | 1 | `res/suzanne.obj/.jpg` |
| 5 | `5_vbo_indexing` | `VBOIndexing.h` | Vertex dedup + index buffer; two monkeys; optional alpha | 1 (2 draws) | suzanne |
| 6 | `6_normal_mapping` | `NormalMapping.h` | Tangent space, diffuse/normal/specular maps | 1 | `res/cylinder/*` |
| 7 | `7_render_to_texture` | `RenderToTexture.h` | Offscreen FBO → full-screen quad post effect | 2 | suzanne |
| 8 | `8_shadow_mapping` | `ShadowMapping.h` | Depth-from-light pass + PCF shadow lookup | 2 | `res/room/room_thickwalls.obj`, `uvmap.jpg` |

All GLSL files are `#version 330 core` and live in `shaders/`, loaded at runtime.

---

## 1. QuadScene — `1_quad`
- **GL objects:** VAO, VBO (interleaved `x,y,r,g,b`; 4 verts), IBO (6 indices). This is the only scene
  whose attributes are recorded in the VAO once, in `onAttach`.
- **Frame:** clear colour (UI-editable), `transform = rotateZ(angle)`, `glDrawElements(6)`.
- **Shaders:** `quad.vert/.frag`, uniform `transform`.
- **State:** `glDisable(GL_DEPTH_TEST)` in `onAttach`, and it is never restored.
- **UI:** clear colour, rotation speed.
- **VK port:** the "hello triangle" equivalent. Vertex/index buffers need staging uploads; the rotation
  becomes a push constant.

## 2. CubeScene — `2_cube`
- **GL objects:** VAO, position VBO (36 verts), colour VBO (36 random colours). No IBO; uses `glDrawArrays(36)`.
- **Frame:** fixed camera at (4,3,-3) looking at the origin, aspect from ImGui DisplaySize; model rotates
  about (0.5,1,0). Uniform `MVP`.
- **Shaders:** `simple.vert/.frag`.
- **State:** depth test on, `GL_LESS`.
- **VK port:** first depth image; UBO or push constant for MVP; `GLM_FORCE_DEPTH_ZERO_TO_ONE` + negative
  viewport height so the image matches.

## 3. UVCubeScene — `3_cube_uv`
- **GL objects:** position VBO, UV VBO (replaced when the texture changes), `Texture`.
- **Frame:** uses `CameraController` (arcball by default); `MVP = VP * rotate(model)`.
- **Shaders:** `simple_uv.vert/.frag`, uniform `MVP`, sampler on unit 0 (default binding; never set
  explicitly). The texture is never bound in `onRender`; it relies on the constructor's bind (see known-issues).
- **UI:** rotation speed, a texture combo (box.jpg / grid.png) that swaps texture and UV set, and the camera panel.
- **VK port:** image upload, layout transitions, sampler, combined image sampler descriptor, mip generation.

## 4. BasicShading — `4_basic_shading`
- **GL objects:** position, normal and UV VBOs from `Mesh(suzanne.obj)`, **not indexed** (2904 verts); `Texture(suzanne.jpg)`.
- **Frame:** uniforms `MVP, V, M, LightPosition_worldspace, LightColor, LightPower, MaterialAmbient,
  MaterialSpecular`; `glDrawArrays`.
- **Shaders:** `standard_shading.vert/.frag`. Lighting is computed in camera space; diffuse = texture,
  falloff = 1/d², specular = `pow(cosAlpha,5)`.
- **Note:** `V` is given the **view-projection** matrix (`getViewProjection()`), not the view matrix, so
  the camera-space lighting is subtly off.
- **UI:** light position, colour and power; ambient; specular; camera panel.
- **VK port:** ~8 uniforms become one UBO struct (std140 layout rules!) plus a descriptor set.

## 5. VBOIndexing — `5_vbo_indexing`
- Same as 4, but `m_Mesh->index()` reduces 2904 vertices to 590, and it adds an `IndexBuffer` and `glDrawElements`.
- Draws suzanne **twice**, the second one translated by +1 in x with halved material strength.
- If `RendererConfig::m_Blend` is on **when the scene is attached**, it uses
  `standard_shading_alpha.frag` (outputs `vec4` with alpha) for simple transparency. No sorting.
- **VK port:** index buffer, two draws with different push constants (or a dynamic UBO, which links to
  Glint_vk's `dynamic_uniform_buffer` sample). Blending is baked into the pipeline, so it needs two pipelines.

## 6. NormalMapping — `6_normal_mapping`
- `Mesh(cylinder.obj)` → `indexWithTangentBasis()` (192 → 66 verts). VBOs for position, UV, normal,
  tangent and bitangent (locations 0–4), plus an IBO.
- Textures: diffuse (unit 0), normal (unit 1) and specular (unit 2) via `Shader::bindTexture`.
- Uniforms: `MVP, V, M, MV3x3`, plus the light and material uniforms as in scene 4.
- **Shaders:** `normal_mapping.vert/.frag`. Light and eye vectors are moved into tangent space in the vertex shader.
- **VK port:** a descriptor set layout with 3 sampled images; tangent attributes are the first real
  multi-attribute vertex input state.

## 7. RenderToTexture — `7_render_to_texture`
- **Pass 1 (offscreen):** `FrameBuffer` sized to the window at setup, with an RGBA8 colour texture, a
  depth *texture*, and `glDrawBuffers({COLOR_ATTACHMENT0})`. Draws the indexed suzanne with standard shading.
- **Pass 2 (screen):** full-screen quad (6 verts, `passthrough.vert` + `wobbly_texture.frag`) sampling
  the colour texture, or the depth texture when "Show Depth Buffer" is on.
- Setup is delayed until ImGui knows the display size (`m_RTTInitialized`, retried in `onRender`).
- **Note:** none of the quad shader's uniforms (`time`, `isDepth`, `depthNear/Far/Scale`) are ever set,
  so the wobble is frozen at `time=0`, the depth view shows the raw depth in the red channel, and the depth
  sliders have no effect. See [known-issues.md](known-issues.md).
- **Note:** the FBO is not recreated on window resize.
- **VK port:** offscreen colour + depth images, two `vkCmdBeginRendering` passes, and an explicit
  **image barrier** between them (colour attachment write → shader read). This is where GL's implicit
  synchronisation becomes visible.

## 8. ShadowMapping — `8_shadow_mapping` (default at startup)
- **Pass 1 (depth from light):** an FBO with a depth texture only (`glDrawBuffer(GL_NONE)`), sized to the
  **window** rather than a fixed shadow resolution.
  - Light projection is either orthographic `(-10,10,-10,10,-10,20)` for a directional light, or a 45°
    perspective (2–50) when "Spotlight" is ticked.
  - `depthMVP = P_light * V_light * I`.
  - Shaders `depth_rtt.vert/.frag`; back-face culling.
- **Pass 2 (camera):** `shadow_mapping.vert/.frag` (4-tap Poisson PCF, `bias=0.005`) or
  `shadow_mapping_simple.*` (a single tap).
  - `DepthBiasMVP = bias * depthMVP`, where the bias matrix maps [-1,1] → [0,1].
  - The shadow map is bound to unit 1 as `sampler2DShadow`.
- **Note:** the depth texture never gets `GL_TEXTURE_COMPARE_MODE = GL_COMPARE_REF_TO_TEXTURE`, so
  sampling it as `sampler2DShadow` is **undefined behaviour** per the GL spec. It happens to work on some drivers.
- **Note:** `LightPosition_worldspace` is set but the shader doesn't use it, so the driver optimises it out
  and the log warns "uniform doesn't exist".
- **UI:**
  - light position, colour and power; ambient; specular
  - toggles: "LightDir is -", Spotlight, Simple
  - "Show Depth Buffer", which draws the shadow map in the panel via `ImGui::Image` with flipped UVs
  - camera panel
- **VK port:** depth-only pipeline with no colour attachments; `vkCmdSetDepthBias` in place of shader bias
  or `glPolygonOffset`; a sampler with `compareEnable = VK_TRUE` (the thing GL forgot here); a barrier from
  depth write to shader read; and the bias matrix must change because VK depth is already [0,1].

---

## Shader index

| Shader(s) | Used by | Key inputs |
|---|---|---|
| `quad.vert/.frag` | 1 | `transform`; attrs 0=vec2 pos, 1=colour |
| `simple.vert/.frag` | 2 | `MVP`; 0=pos, 1=colour |
| `simple_uv.vert/.frag` | 3 | `MVP`, sampler; 0=pos, 1=uv |
| `standard_shading.vert/.frag` | 4, 5, 7 | `MVP,V,M`, light ×3, material ×2, sampler |
| `standard_shading_alpha.frag` | 5 (blend on) | as above, `vec4` output |
| `normal_mapping.vert/.frag` | 6 | + `MV3x3`, 3 samplers; attrs 0–4 |
| `passthrough.vert` + `wobbly_texture.frag` | 7 | `renderedTexture`, `time`, `isDepth`, depth remap uniforms |
| `depth_rtt.vert/.frag` | 8 (pass 1) | `depthMVP` |
| `shadow_mapping.vert/.frag` | 8 (pass 2) | `MVP,V,M,DepthBiasMVP`, `LightInvDirection_worldspace`, `sampler2DShadow shadowMap` |
| `shadow_mapping_simple.vert/.frag` | 8 (pass 2, "Simple") | same, single tap |
