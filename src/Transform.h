//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Transform.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  03-07-2022
// 
// PURPOSE:  Hold position, rotation, and scale in 3D space.
// 
//---------------------------------------------------------------------------// 

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Nelson.h"

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	Transform(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1)) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}
};

#endif /* TRANSFORM_H */
