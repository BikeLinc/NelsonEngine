/******************************************************************************
 *
 * @file	Transform.h
 *
 * @author	Lincoln Scheer
 * @since	03/07/2022
 *
 * @brief	Transform is a struct that holds all positional, rotational,
 *		and scale data per model.
 *****************************************************************************/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "NelsonEngine.h"

struct Transform {
	glm::vec3 position, rotation, scale;
	Transform(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1)) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}
};

#endif /* TRANSFORM_H */
