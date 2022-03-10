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
		glEnable(GL_DEPTH_TEST);
		

	}

	void render(Scene* scene) {
		
		glClearColor(scene->color.x, scene->color.y, scene->color.z, scene->color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < scene->models.size(); i++) {
			scene->models.at(i)->draw(scene->offset);
		}
	}
};

#endif // !RENDERER_H
