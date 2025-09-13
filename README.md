Nelson is a game engine and editor that is currently under development. The purpose of this project is to create a small and portable game engine as a playground for prototyping games.

## Requirements
Supports `macOS` and `debian` based linux distros.

## Dependencies
- [GLEW](https://github.com/nigels-com/glew)
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [SOIL2](https://github.com/SpartanJ/SOIL2)
- [IMGUI](https://github.com/ocornut/imgui)

## Architecture
The engine architecture is largely inspired by the brilliant interface that [Unity](https://unity.com/) uses. The name of the game is clean class design.

## Quick Start on Linux
```
sudo apt install cmake build-essential libglew-dev libglfw3-dev libglm-dev libxinerama-dev libxcursor-dev libxi-dev 
mkdir -p build
cd build
cmake ..
make
```

## Current UML Diagram
![UML](https://github.com/BikeLinc/NelsonEngine/blob/main/engine-uml-030822.png)
