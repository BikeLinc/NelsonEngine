/******************************************************************************
 *
 * @file	Scene.h
 *
 * @author	Lincoln Scheer
 * @since	02/23/2022
 *
 * @brief	Scene is the class that Nelson uses to describe where and how 
 *		to render an object.
 *
 *****************************************************************************/

#ifndef SCENE_H
#define SCENE_H

#include "NelsonEngine.h"

struct Scene {
	glm::vec4 backgroundColor;
	std::vector<Model*> models;

	Scene(glm::vec4 backgroundColor = glm::vec4(1)) {
		this->backgroundColor = backgroundColor;
	}

	void add(Model& model) {
		models.push_back(&model);
	}

	void update(double delta) {
		for (int i = 0; i < models.size(); i++) {
			models.at(i)->update(delta);
		}
	}

	void destroy() {
		for (int i = 0; i < models.size(); i++) {
			models.at(i)->destroy();
		}
	}
};

#endif
