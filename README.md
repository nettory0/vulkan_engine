# Vulkan Engine

A personal real-time rendering engine written in modern C++ (C++20) on top of the Vulkan API.

<!-- TODO: add screenshots / GIF here, e.g. docs/screenshot_raster.png, docs/screenshot_rt.png -->

## Features

- **Vulkan core layer** (`vulkan_struct`) — RAII wrappers for Vulkan objects built on a single
  `CVulkanObject<Handle, Device, CreateInfo>` template, create-info builders, function loading via volk
- **Rendering**
  - Dynamic rendering (`VK_KHR_dynamic_rendering`) and Synchronization2 barriers
  - Multiple frames in flight, swapchain recreation on resize
  - Static and skinned meshes
- **Hardware ray tracing** (`VK_KHR_ray_tracing_pipeline`) — raygen / closest-hit / miss / shadow shaders,
  glTF PBR material model
- **Compute** — GPU ray picking for object selection
- **Skeletal animation** — loading and playback of animated models (assimp)
- **Text rendering** — FreeType glyph outlines vectorised and triangulated into meshes
- **2D UI layer** and in-engine menu
- **Scene / component system**, orbit camera, physics-driven character controller, selection and highlighting
- **Multiple front-ends** — GLFW, SDL and Qt 6 windows share the same engine library

## Project structure

| Module | Description |
|---|---|
| `vulkan_struct` | Low-level Vulkan wrappers and struct builders |
| `vulkan_engine` | Render system, pipelines, descriptors, ray tracing, compute, animation, scene |
| `asset_loader` | Model and scene description loading (YAML) |
| `text` | Glyph vectorisation and triangulation |
| `physic_engine` | Simple physics |
| `interface_menu` | 2D UI / menu |
| `commons` | Shared types, geometry primitives, logging |
| `shaders` | GLSL shaders, compiled to SPIR-V at build time |
| `window_app` / `sdl_app` / `qt_gui_render` | GLFW / SDL / Qt applications |

## Building

Requirements:

- CMake 3.15+
- C++20 compiler (tested with Visual Studio 2022)
- [Vulkan SDK](https://vulkan.lunarg.com/) 1.4 (`glslangValidator` is used to compile shaders)
- Qt 6 (Core, Widgets) for the `qt_gui_render` application
- A GPU with ray tracing support for the ray-traced mode

Other dependencies (volk, VMA, GLFW, SDL, assimp, FreeType, stb, glm, yaml-cpp) are downloaded
automatically with CMake `FetchContent`.

```bash
cmake -S CMakeSolution -B build
cmake --build build --config Release
```

Executables and compiled shaders are placed in `build/bin`.

<!-- TODO: describe where to get the models (models/...) used by the demo scenes -->

## Controls

| Key | Action |
|---|---|
| W / A / S / D | Move the character |
| Space | Jump |
| Left mouse | Select object |
| Ctrl + left mouse | Add to selection |
| Esc | Exit (GLFW app) |

## Third-party code

This repository includes code derived from the following projects; the original copyright notices
are kept in the corresponding files and the full license texts are in [`third_party_licenses/`](third_party_licenses/):

- **[Poly2Tri](https://github.com/jhasse/poly2tri)** — constrained Delaunay triangulation
  (`text/tg_sweep*`, `text/tg_shapes*`, `text/tg_utils.h`), BSD 3-Clause
- **[FTGL](https://github.com/frankheckenbach/ftgl)** — glyph outline vectoriser
  (`text/tg_vectoriser*`), MIT
- **[NVIDIA nvpro-samples](https://github.com/nvpro-samples)** — PBR, random and ray tracing shader
  helpers (`shaders/common/`, `shaders/Ray2/pbr_gltf.glsl`), Apache 2.0

Libraries fetched at build time are distributed under their own licenses.

## License

The engine code is licensed under the [MIT License](LICENSE).
