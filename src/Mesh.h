/******************************************************************************
 *
 * @file 	Mesh.h
 *
 * @author 	Lincoln Scheer
 * @since 	03/07/2022
 *
 * @brief 	Mesh is a struct whos constructor takes in geometry data ans it 
 *		is converted into renderable OpenGL data VAO, VBO, and EBO;
 *
 *****************************************************************************/
#ifndef MESH_H
#define MESH_H

#include "NelsonEngine.h"

template <class T>
inline void glBufferDataFromVector(GLenum target, const std::vector<T>& v, GLenum usage) {
	glBufferData(target, v.size() * sizeof(T), &v[0], usage);
}

struct Mesh {
	unsigned int VBO, VAO, EBO;
	Geometry geometry;
	Mesh(Geometry geometry) {
		this->geometry = geometry;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferDataFromVector(GL_ARRAY_BUFFER, geometry.vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferDataFromVector(GL_ELEMENT_ARRAY_BUFFER, geometry.indices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
};

#endif /* MESH_H */
