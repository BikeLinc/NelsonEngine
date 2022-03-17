/******************************************************************************
 *
 * @file	Model.h
 *
 * @author	Lincoln Scheer
 * @since	03/07/2022
 *
 * @brief	Model is a struct that combines a texture, transformation, mesh, and a
 *		shader in order to create a abstract object that holds renderable data.
 *
 *****************************************************************************/

#ifndef MODEL_H
#define MODEL_H

#include "Nelson.h"

struct Model {
	const char* name;
	const char* texturePath;
	unsigned int textureID = 0;
	glm::vec2 meshBounds;
	Transform transform;
	Mesh* mesh;
	Shader* shader;
	int order = 0;

	Model(const char* name, const char* texturePath, Geometry geometry, Transform transformOrigin) {
		this->name = name;
		this->texturePath = texturePath;
		textureID = TextureLoader(texturePath).getTexture();
		this->transform = transformOrigin;
		mesh = new Mesh(geometry);
		shader = new Shader("../res/shaders/vert.glsl", "../res/shaders/frag.glsl");
		shader->use();
		shader->setInt("textureSRC", textureID);
	}

	Model(const char* name, const char* texturePath, Transform transformOrigin) {
		this->name = name;
		this->texturePath = texturePath;
		TextureLoader texLoader(texturePath);
		textureID = texLoader.getTexture();
		int max = std::max(texLoader.width, texLoader.height);
		this->meshBounds = glm::vec2(texLoader.width / max, texLoader.height / max);
		this->transform = transformOrigin;
		mesh = new Mesh(PlaneGeometry(meshBounds));
		shader = new Shader("../res/shaders/vert.glsl", "../res/shaders/frag.glsl");
		shader->use();
		shader->setInt("textureSRC", textureID);
	}

	virtual void update(double delta) {

	}

	void draw(Transform offset) {
		glActiveTexture(GL_TEXTURE0 + textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// create transformations
		glm::mat4 model_transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model_transform = glm::translate(model_transform, transform.position + offset.position);
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model_transform = glm::scale(model_transform, transform.scale * offset.scale);

		// get matrix's uniform location and set matrix
		shader->use();
		unsigned int transformLoc = glGetUniformLocation(shader->ID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_transform));

		// render container
		glBindVertexArray(mesh->VAO);
		glDrawElements(GL_TRIANGLES, mesh->geometry.indices.size(), GL_UNSIGNED_INT, 0);
	}

	void destroy() {
		glDeleteVertexArrays(1, &mesh->VAO);
		glDeleteBuffers(1, &mesh->VBO);
		glDeleteBuffers(1, &mesh->EBO);
	}
};

#endif /* MODEL_H */
