//
//  _   _      _                       _____             _            
// | \ | | ___| |___  ___  _ __       | ____|_ __   __ _(_)_ __   ___ 
// |  \| |/ _ \ / __|/ _ \| '_ \ _____|  _| | '_ \ / _` | | '_ \ / _ \
// | |\  |  __/ \__ \ (_) | | | |_____| |___| | | | (_| | | | | |  __/
// |_| \_|\___|_|___/\___/|_| |_|     |_____|_| |_|\__, |_|_| |_|\___|
//                                                 |___/              
// 
// NelsonEngine v1.0 - Copyright(c) 2022 Lincoln Scheer
// 
// MIT License
// 
// Nelson Engine is a 2D and 3D game engine. Since this engine is being written
// in C++, the engine will need to be build for the user's operating systems. 
// Development is being conducted on Windows and OSX os's and should be
// compatible with Linux systems with minor tweaks to the build files. 
// 
// Please visit https://github.com/bikelinc/nelsonengine for all development
// related things.
// 
//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Nelson.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  03-16-2022
// 
// PURPOSE:  Header file for all Nelson Engine includes.
// 
//---------------------------------------------------------------------------// 

// PREPROCESSOR
#pragma once
#define GL_SILENCE_DEPRECATION

// STL LIBRARY
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <functional>
#include <queue>

// GLEW - OPENGL EXTENSION WRANGLER INCLUDES
#include <GL/glew.h>
#define  GLEW_STATIC

// GLFW - OPENGL WINDOW FRAMEWORK INCLUDES
#include <GLFW/glfw3.h>

// SOIL2 - SOIL2 IMAGE LOADER INCLUDES
#include "../vendor/SOIL2/SOIL2.h"

// GLM - OPENGL MATH INCLUDES
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"

// LOADERS
#include "Shader.h"
#include "TextureLoader.h"

// GAME OBJECTS
#include "Transform.h"
#include "Geometry.h"
#include "Mesh.h"
#include "Model.h"
#include "Scene.h"

// SYSTEMS
#include "MessageSystem.h"
#include "Editor.h"
#include "Renderer.h"
#include "Window.h"