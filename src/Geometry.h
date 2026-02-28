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

#include "Nelson.h"

struct Geometry {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};

inline void appendVertex(std::vector<float>& vertices, const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv) {
	vertices.push_back(position.x);
	vertices.push_back(position.y);
	vertices.push_back(position.z);
	vertices.push_back(normal.x);
	vertices.push_back(normal.y);
	vertices.push_back(normal.z);
	vertices.push_back(uv.x);
	vertices.push_back(uv.y);
}

struct PlaneGeometry : public Geometry {
	glm::vec2 size;
	PlaneGeometry(glm::vec2 size) {
		float width2 = size.x / 2, height2 = size.y / 2;
		std::vector<float> verts;
		verts.reserve(4 * 8);
		const glm::vec3 n(0.0f, 0.0f, 1.0f);
		appendVertex(verts, glm::vec3(width2, height2, 0.0f), n, glm::vec2(1.0f, 1.0f));
		appendVertex(verts, glm::vec3(width2, -height2, 0.0f), n, glm::vec2(1.0f, 0.0f));
		appendVertex(verts, glm::vec3(-width2, -height2, 0.0f), n, glm::vec2(0.0f, 0.0f));
		appendVertex(verts, glm::vec3(-width2, height2, 0.0f), n, glm::vec2(0.0f, 1.0f));

		this->vertices = verts;

		std::vector<unsigned int> indic = {
		    0, 1, 3, // first triangle
		    1, 2, 3  // second triangle
		};

		this->indices = indic;
	}
};

struct CubeGeometry : public Geometry {
        CubeGeometry() {
                std::vector<float> verts =
                {
                                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
                                 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
                                 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
                                 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
                                -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
                                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

                                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
                                 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
                                 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
                                 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
                                -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
                                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

                                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
                                -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
                                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
                                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
                                -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
                                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

                                 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
                                 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
                                 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
                                 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
                                 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
                                 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

                                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
                                 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
                                 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
                                 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
                                -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
                                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

                                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
                                 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
                                 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
                                 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
                                -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
                                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
                };

                std::vector<unsigned int> indic =
                {
                        0,1,2,
                        3,4,5,
                        6,7,8,
                        9,10,11,
                        12,13,14,
                        15,16,17,
                        18,19,20,
                        21,22,23,
                        24,25,26,
                        27,28,29,
                        30,31,32,
                        33,34,35
                };

                this->vertices = verts;

                this->indices = indic;
        }
	};

struct SphereGeometry : public Geometry {
	SphereGeometry(float radius = 0.5f, unsigned int stacks = 16, unsigned int sectors = 32) {
		const float pi = 3.14159265358979323846f;
		if (stacks < 3) {
			stacks = 3;
		}
		if (sectors < 3) {
			sectors = 3;
		}

		vertices.reserve((stacks + 1) * (sectors + 1) * 8);
		indices.reserve(stacks * sectors * 6);

		for (unsigned int stack = 0; stack <= stacks; ++stack) {
			const float v = static_cast<float>(stack) / static_cast<float>(stacks);
			const float phi = pi * v;
			const float sinPhi = std::sin(phi);
			const float cosPhi = std::cos(phi);

			for (unsigned int sector = 0; sector <= sectors; ++sector) {
				const float u = static_cast<float>(sector) / static_cast<float>(sectors);
				const float theta = 2.0f * pi * u;
				const float sinTheta = std::sin(theta);
				const float cosTheta = std::cos(theta);

				const float x = radius * sinPhi * cosTheta;
				const float y = radius * cosPhi;
				const float z = radius * sinPhi * sinTheta;
				glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
				vertices.push_back(normal.x);
				vertices.push_back(normal.y);
				vertices.push_back(normal.z);
				vertices.push_back(u);
				vertices.push_back(1.0f - v);
			}
		}

		const unsigned int rowSize = sectors + 1;
		for (unsigned int stack = 0; stack < stacks; ++stack) {
			for (unsigned int sector = 0; sector < sectors; ++sector) {
				const unsigned int first = stack * rowSize + sector;
				const unsigned int second = first + rowSize;

				indices.push_back(first);
				indices.push_back(second);
				indices.push_back(first + 1);

				indices.push_back(second);
				indices.push_back(second + 1);
				indices.push_back(first + 1);
			}
		}
	}
};

#endif /* GEOMETRY_H */
