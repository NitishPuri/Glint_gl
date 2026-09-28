# Glint_gl — Documentation

Architecture notes for Glint_gl, a small OpenGL 4.6 learning renderer built around a scene browser.
Written 2026-09-29 while porting to Linux and preparing the migration into
[`glint`](../../glint) (OpenGL + Vulkan side by side).

| Doc | What's in it |
|---|---|
| [architecture.md](architecture.md) | Big picture: layers, startup, frame loop, ownership, GL state model, paths, logging and debugging |
| [modules.md](modules.md) | Reference for each class and file in `src/Core` and `src/Graphics` |
| [scenes.md](scenes.md) | The 8 scenes: technique, passes, GL objects, shaders, UI, and notes for porting to Vulkan |
| [build.md](build.md) | Building on Linux and Windows, dependencies, what the Linux port changed |
| [known-issues.md](known-issues.md) | Bugs and tech debt found during review, with file:line references |

## One-paragraph summary

`app.cpp` creates a GLFW window with a GL 4.6 core context, an ImGui layer and a `SceneManager`. Each
technique is a `SceneBase` subclass defined in a single header in `src/Scenes/`, registered by name with a
factory lambda. Selecting a scene in the ImGui combo **constructs a new instance** and calls `onAttach()`,
which loads shaders, meshes and textures. After that, every frame calls `onUpdate → onRender → onImGuiRender`.
Scenes talk to OpenGL directly, with a few thin RAII wrappers (`Shader`, `VertexBuffer`, `IndexBuffer`,
`VertexArray`, `Texture`, `FrameBuffer`). There is no renderer abstraction, material system or scene graph;
each scene owns its GL objects and sets global GL state itself.
