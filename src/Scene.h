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

#include "Nelson.h"

struct Scene {
	std::string name;
	glm::vec4 color;
	std::vector<Model*> models;
	Transform offset;

	Scene(std::string name = "scene", glm::vec4 color = glm::vec4(1)) {
		this->name = name;
		this->color = color;
	}

	void add(Model* model) {
		models.push_back(model);
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
