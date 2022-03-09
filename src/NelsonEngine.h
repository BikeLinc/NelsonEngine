/******************************************************************************
 *
 * @file	NelsonEngine.h
 *
 * @author	Lincoln Scheer
 * @since	02/21/2022
 *
 * @breif	NelsonEngine.h is a static header-only collection of the headers
 *		that Nelson requires. Grouping of includes in this file will allow
 *		an easy transition to using pre-compiled headers.
 *
 *****************************************************************************/

#ifndef NELSON_ENGINE_H
#define NELSON_ENGINE_H

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
