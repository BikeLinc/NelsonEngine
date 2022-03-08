/******************************************************************************
 *
 * @file	Geometry.h
 *
 * @author	Lincoln Scheer
 * @since	03/07/2022
 *
 * @brief	Geometry is a combination of two std::vectors for vertices and
 *		indices of a model's geometry. Geometry can be extended to 
 *		create custom primitives.
 *
 *****************************************************************************/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "NelsonEngine.h"

struct Geometry {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};

struct PlaneGeometry : public Geometry {
	glm::vec2 size;
	PlaneGeometry(glm::vec2 size) {
		float width2 = size.x / 2, height2 = size.y / 2;
		std::vector<float> verts = {
			// positions          // texture coords
			 width2,  height2, 0.0f,   -1.0, -1.0, // top right
			 width2, -height2, 0.0f,   -1.0, 0.0f, // bottom right
			-width2, -height2, 0.0f,   0.0f, 0.0f, // bottom left
			-width2,  height2, 0.0f,   0.0f, -1.0  // top left
		};

		this->vertices = verts;

		std::vector<unsigned int> indic = {
		    0, 1, 3, // first triangle
		    1, 2, 3  // second triangle
		};

		this->indices = indic;
	}
};

#endif /* GEOMETRY_H */
