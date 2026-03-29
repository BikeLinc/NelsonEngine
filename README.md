<p align="center">
  <img src="res/images/nelson-readme.png" alt="Nelson Engine logo" width="128" />
</p>

<h1 align="center">Nelson Engine</h1>

<p align="center">
  A lightweight C++ game engine and editor for rapid prototyping.
</p>

<p align="center">
  Built with OpenGL, CMake, and an intentionally small, portable codebase.
</p>


## Overview
The editor/runtime follows a lightweight ECS-style flow with JSON scene I/O.

![Nelson Architecture Diagram](docs/architecture.png)

## Features
- Load and display 3D models
- Basic scene management with load/save support
- Simple editor UI for scene properties
- Real-time rendering with OpenGL

## Requirements
Currently supports `macOS` and Debian-based Linux distributions.

- C++17 compiler (`g++` or `clang++`)
- CMake 3.1 or newer
- OpenGL development libraries

## Dependencies
Most dependencies are vendored directly in the repository:

- [GLEW](https://github.com/nigels-com/glew) for OpenGL extension loading
- [GLFW](https://github.com/glfw/glfw) for windowing, input, and context creation
- [GLM](https://github.com/g-truc/glm) for math utilities
- [SOIL2](https://github.com/SpartanJ/SOIL2) for texture and image loading
- [Dear ImGui](https://github.com/ocornut/imgui) for the editor UI
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) for OBJ and MTL model loading

Scene serialization currently uses an internal parser/writer:

- `src/SimpleJson.h` for lightweight JSON parsing and writing without an external JSON dependency

## Project Layout
Project-specific content lives under `projects/<project-name>/`:

```text
projects/
  default/
    project.json
    scenes/
      default.scene.json
    assets/
      models/
        sponza/
```

Shared fallback assets remain in `res/` for common shaders, images, and legacy scenes.

## Quick Start on Linux
Install system packages:

```bash
sudo apt install cmake build-essential libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev libxinerama-dev libxcursor-dev libxi-dev
```

Configure and build:

```bash
mkdir -p build
cd build
cmake ..
make
```

Run from the project root:

```bash
./Nelson
```

Run with an explicit startup scene JSON:

```bash
./Nelson projects/default/scenes/default.scene.json
```
