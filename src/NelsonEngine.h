//
//  _   _      _                       _____             _            
// | \ | | ___| |___  ___  _ __       | ____|_ __   __ _(_)_ __   ___ 
// |  \| |/ _ \ / __|/ _ \| '_ \ _____|  _| | '_ \ / _` | | '_ \ / _ \
// | |\  |  __/ \__ \ (_) | | | |_____| |___| | | | (_| | | | | |  __/
// |_| \_|\___|_|___/\___/|_| |_|     |_____|_| |_|\__, |_|_| |_|\___|
//                                                 |___/              
// 
// NelsonEngine v1.0 - 2022
// 
// Nelson Engine is a 2D and 3D game engine. Since this engine is being written
// in C++, the engine will need to be build for the user's operating systems. 
// Development is being conducted on Windows and OSX os's and should be
// compatible with Linux systems with minor tweaks to the build files. 
// 
// 
//--[Nelson.h]---------------------------------------------------//
// 
// 
// FILE:     Nelson.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  03-16-2022
// 
// PURPOSE:  Header file for all Nelson Engine includes.
// 
//
//---------------------------------------------------------------------------// 
#ifndef NELSON_ENGINE_H
#define NELSON_ENGINE_H
#define GL_SILENCE_DEPRECATION

// C++ Standard Library
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// Libraries
#include <GL/glew.h>
#define GLEW_STATIC
#include <GLFW/glfw3.h>
#include "../vendor/SOIL2/SOIL2.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"

// Nelson Utils
#include "MessageSystem.h"
#include "Shader.h"
#include "TextureLoader.h"

// Nelson Objects
#include "Transform.h"
#include "Geometry.h"
#include "Mesh.h"
#include "Model.h"
#include "Scene.h"

// Nelson Engine
#include "Editor.h"
#include "Window.h"
#include "Renderer.h"


#endif /* NELSON_ENGINE_H */