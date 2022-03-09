/******************************************************************************
 *
 * @file	Renderer.h
 *
 * @author	Lincoln Scheer
 * @since	03/07/2022
 *
 * @brief	Renderer is a class that performs all drawing of renderables.
 *
 *****************************************************************************/

#ifndef RENDERER_H

#include "NelsonEngine.h"

class Renderer {
public:
	void init() {

	}

	void render(Scene* scene) {
		
		glClearColor(scene->backgroundColor.x, scene->backgroundColor.y, scene->backgroundColor.z, scene->backgroundColor.w);
		glClear(GL_COLOR_BUFFER_BIT);
		for (int i = 0; i < scene->models.size(); i++) {
			scene->models.at(i)->draw(scene->offset);
		}
	}
};

#endif // !RENDERER_H
