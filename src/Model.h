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

#include "tinyobjloader/tiny_obj_loader.h"
#include <map>

struct ModelLightData {
	int type = 1;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float range = 1.0f;
};

constexpr int MODEL_MAX_LIGHTS = 8;

inline glm::vec3 ModelCameraPosition() {
	return glm::vec3(0.0f, 1.2f, 3.5f);
}

inline glm::vec3 ModelCameraTarget() {
	return glm::vec3(0.0f, 0.8f, -2.0f);
}

inline glm::mat4 ModelViewMatrix() {
	return glm::lookAt(ModelCameraPosition(), ModelCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));
}

inline glm::mat4 ModelProjectionMatrix(float aspect) {
	return glm::perspective(glm::radians(60.0f), aspect, 0.01f, 500.0f);
}

struct ModelDrawSettings {
	Transform offset;
	glm::vec4 tint = glm::vec4(1.0f);
	float metallic = 0.0f;
	float roughness = 1.0f;
	bool wireframe = false;
	bool fullBrightOverride = false;
	glm::vec3 ambientColor = glm::vec3(1.0f);
	float ambientIntensity = 0.2f;
	glm::vec3 cameraWorldPosition = ModelCameraPosition();
	glm::mat4 view = ModelViewMatrix();
	glm::mat4 projection = ModelProjectionMatrix(16.0f / 9.0f);
	const std::vector<ModelLightData>* lights = nullptr;
};

struct Model {
	struct Submesh {
		Mesh* mesh = nullptr;
		unsigned int textureID = 0;
	};

	std::string ownedName;
	std::string ownedTexturePath;
	const char* name;
	const char* texturePath;
	unsigned int textureID = 0;
	glm::vec2 meshBounds;
	Transform transform;
	Mesh* mesh = nullptr;
	Shader* shader = nullptr;
	std::vector<Submesh> submeshes;
	int order = 0;
	std::string sourceType = "unknown"; // "obj" | "plane"
	std::string sourceObjPath;
	std::string sourceMtlDir;
	glm::vec2 sourcePlaneSize = glm::vec2(1.0f);

	Model(const char* name, const char* texturePath, Geometry geometry, Transform transformOrigin) {
		ownedName = name ? name : "Model";
		this->name = ownedName.c_str();
		ownedTexturePath = texturePath ? texturePath : "";
		this->texturePath = ownedTexturePath.c_str();
		textureID = TextureLoader(this->texturePath).getTexture();
		this->transform = transformOrigin;
		std::string vertPath = resolveExistingPath({ "res/shaders/vert.glsl", "../res/shaders/vert.glsl" });
		std::string fragPath = resolveExistingPath({ "res/shaders/frag.glsl", "../res/shaders/frag.glsl" });
		shader = new Shader(vertPath.c_str(), fragPath.c_str());
		setSingleMesh(geometry, textureID);
	}

	Model(const char* name, const char* texturePath, Transform transformOrigin) {
		ownedName = name ? name : "Model";
		this->name = ownedName.c_str();
		ownedTexturePath = texturePath ? texturePath : "";
		this->texturePath = ownedTexturePath.c_str();
		TextureLoader texLoader(this->texturePath);
		textureID = texLoader.getTexture();
		int max = std::max(texLoader.width, texLoader.height);
		this->meshBounds = glm::vec2(texLoader.width / max, texLoader.height / max);
		this->transform = transformOrigin;
		sourceType = "plane";
		sourcePlaneSize = meshBounds;
		std::string vertPath = resolveExistingPath({ "res/shaders/vert.glsl", "../res/shaders/vert.glsl" });
		std::string fragPath = resolveExistingPath({ "res/shaders/frag.glsl", "../res/shaders/frag.glsl" });
		shader = new Shader(vertPath.c_str(), fragPath.c_str());
		setSingleMesh(PlaneGeometry(meshBounds), textureID);
	}

	Model(const char* name, const char* texturePath, glm::vec2 planeSize, Transform transformOrigin) {
		ownedName = name ? name : "Model";
		this->name = ownedName.c_str();
		ownedTexturePath = texturePath ? texturePath : "";
		this->texturePath = ownedTexturePath.c_str();
		TextureLoader texLoader(this->texturePath);
		textureID = texLoader.getTexture();
		this->meshBounds = planeSize;
		this->transform = transformOrigin;
		sourceType = "plane";
		sourcePlaneSize = planeSize;
		std::string vertPath = resolveExistingPath({ "res/shaders/vert.glsl", "../res/shaders/vert.glsl" });
		std::string fragPath = resolveExistingPath({ "res/shaders/frag.glsl", "../res/shaders/frag.glsl" });
		shader = new Shader(vertPath.c_str(), fragPath.c_str());
		setSingleMesh(PlaneGeometry(meshBounds), textureID);
	}

	// Constructor for OBJ loading (no texture initially)
	Model(const char* name, Transform transformOrigin) {
		ownedName = name ? name : "Model";
		this->name = ownedName.c_str();
		ownedTexturePath = "";
		this->texturePath = nullptr;
		this->textureID = getFallbackTexture();
		this->transform = transformOrigin;
		sourceType = "obj";
		std::string vertPath = resolveExistingPath({ "res/shaders/vert.glsl", "../res/shaders/vert.glsl" });
		std::string fragPath = resolveExistingPath({ "res/shaders/frag.glsl", "../res/shaders/frag.glsl" });
		shader = new Shader(vertPath.c_str(), fragPath.c_str());
	}

	virtual void update(double delta) {

	}

	// Helper: load OBJ/MTL after construction
	bool LoadOBJ(const char* objPath, const char* mtlBaseDir) {
		sourceType = "obj";
		sourceObjPath = objPath ? objPath : "";
		sourceMtlDir = mtlBaseDir ? mtlBaseDir : "";
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath, mtlBaseDir);
		if (!ret) {
			std::cerr << "Failed to load OBJ: " << err << std::endl;
			return false;
		}
		(void)warn; // Intentionally suppressed in MVP mode to keep startup logs clean.

		std::map<int, Geometry> geometryByMaterial;
		bool hasFaces = false;

		for (const auto& shape : shapes) {
			size_t index_offset = 0;
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				const int fv = shape.mesh.num_face_vertices[f];
				if (fv != 3) {
					std::cerr << "Unsupported OBJ in " << objPath << ": non-triangulated faces are not supported." << std::endl;
					return false;
				}

				int materialId = -1;
				if (f < shape.mesh.material_ids.size()) {
					materialId = shape.mesh.material_ids[f];
				}

				Geometry& geometry = geometryByMaterial[materialId];
				glm::vec3 facePositions[3];
				glm::vec2 faceUVs[3];
				glm::vec3 faceNormals[3];
				bool missingNormal = false;
				for (size_t v = 0; v < 3; v++) {
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					if (idx.vertex_index < 0 || idx.texcoord_index < 0) {
						std::cerr << "Unsupported OBJ in " << objPath << ": UV coordinates are required for all vertices." << std::endl;
						return false;
					}

					const size_t vertexBase = static_cast<size_t>(3) * static_cast<size_t>(idx.vertex_index);
					const size_t texBase = static_cast<size_t>(2) * static_cast<size_t>(idx.texcoord_index);
					if (vertexBase + 2 >= attrib.vertices.size() || texBase + 1 >= attrib.texcoords.size()) {
						std::cerr << "Invalid OBJ index data in " << objPath << "." << std::endl;
						return false;
					}

					const float vx = attrib.vertices[vertexBase + 0];
					const float vy = attrib.vertices[vertexBase + 1];
					const float vz = attrib.vertices[vertexBase + 2];
					const float tx = attrib.texcoords[texBase + 0];
					const float ty = 1.0f - attrib.texcoords[texBase + 1];
					facePositions[v] = glm::vec3(vx, vy, vz);
					faceUVs[v] = glm::vec2(tx, ty);

					if (idx.normal_index >= 0) {
						const size_t normalBase = static_cast<size_t>(3) * static_cast<size_t>(idx.normal_index);
						if (normalBase + 2 < attrib.normals.size()) {
							faceNormals[v] = glm::normalize(glm::vec3(
								attrib.normals[normalBase + 0],
								attrib.normals[normalBase + 1],
								attrib.normals[normalBase + 2]));
						} else {
							missingNormal = true;
						}
					} else {
						missingNormal = true;
					}
				}

				if (missingNormal) {
					const glm::vec3 edgeA = facePositions[1] - facePositions[0];
					const glm::vec3 edgeB = facePositions[2] - facePositions[0];
					const glm::vec3 faceNormal = glm::normalize(glm::cross(edgeA, edgeB));
					for (int v = 0; v < 3; ++v) {
						faceNormals[v] = faceNormal;
					}
				}

				for (int v = 0; v < 3; ++v) {
					const unsigned int nextIndex = static_cast<unsigned int>(geometry.vertices.size() / 8);
					appendVertex(geometry.vertices, facePositions[v], faceNormals[v], faceUVs[v]);
					geometry.indices.push_back(nextIndex);
				}
				index_offset += fv;
				hasFaces = true;
			}
		}

		if (!hasFaces) {
			std::cerr << "Failed to load OBJ: no triangle faces found in " << objPath << "." << std::endl;
			return false;
		}

		clearSubmeshes();
		size_t totalVertices = 0;
		size_t totalIndices = 0;
		for (const auto& entry : geometryByMaterial) {
			const int materialId = entry.first;
			const Geometry& geometry = entry.second;
			if (geometry.indices.empty()) {
				continue;
			}

			unsigned int materialTexture = getFallbackTexture();
			if (materialId >= 0 && static_cast<size_t>(materialId) < materials.size()) {
				const tinyobj::material_t& material = materials[materialId];
				if (!material.diffuse_texname.empty()) {
					std::string texPath = std::string(mtlBaseDir);
					if (!texPath.empty() && texPath.back() != '/' && texPath.back() != '\\') {
						texPath.push_back('/');
					}
					std::string diffusePath = material.diffuse_texname;
					std::replace(diffusePath.begin(), diffusePath.end(), '\\', '/');
					texPath += diffusePath;

					TextureLoader loader(texPath.c_str());
					if (loader.width > 0 && loader.height > 0) {
						materialTexture = loader.getTexture();
					} else {
						std::cout << "Missing diffuse texture '" << texPath << "', using fallback texture." << std::endl;
					}
				}
			}

			Submesh submesh;
			submesh.mesh = new Mesh(geometry);
			submesh.textureID = materialTexture;
			submeshes.push_back(submesh);
			totalVertices += geometry.vertices.size() / 8;
			totalIndices += geometry.indices.size();
		}

		if (submeshes.empty()) {
			std::cerr << "Failed to build renderable mesh data from " << objPath << "." << std::endl;
			return false;
		}

		mesh = submeshes[0].mesh;
		textureID = submeshes[0].textureID;
		std::cout << "Loaded OBJ with " << submeshes.size() << " submeshes, "
			<< totalVertices << " vertices, " << totalIndices << " indices." << std::endl;

		return true;
	}

	void draw(const ModelDrawSettings& settings) {
		// Comprehensive safety checks
		if (!shader) return;
		if (transform.scale.x <= 0.0f || transform.scale.y <= 0.0f || transform.scale.z <= 0.0f) return;
		if (std::isnan(transform.position.x) || std::isnan(transform.position.y) || std::isnan(transform.position.z)) return;
		if (std::isnan(transform.rotation.x) || std::isnan(transform.rotation.y) || std::isnan(transform.rotation.z)) return;
		if (std::isnan(transform.scale.x) || std::isnan(transform.scale.y) || std::isnan(transform.scale.z)) return;
		if (std::isnan(settings.offset.position.x) || std::isnan(settings.offset.position.y) || std::isnan(settings.offset.position.z)) return;
		if (std::isnan(settings.offset.scale.x) || std::isnan(settings.offset.scale.y) || std::isnan(settings.offset.scale.z)) return;
		if (settings.offset.scale.x <= 0.0f || settings.offset.scale.y <= 0.0f || settings.offset.scale.z <= 0.0f) return;
		
		shader->use();
		
		// Bind texture
		if (textureID > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID);
			shader->setInt("textureSRC", 0);
		}

		// create transformations - FIXED ORDER: Scale -> Rotate -> Translate
		glm::mat4 model_transform = glm::mat4(1.0f);
		
		// Apply transforms in correct order
		model_transform = glm::translate(model_transform, transform.position + settings.offset.position);
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model_transform = glm::rotate(model_transform, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model_transform = glm::scale(model_transform, transform.scale * settings.offset.scale);

		shader->setMat4("transform", model_transform);
		shader->setMat4("view", settings.view);
		shader->setMat4("projection", settings.projection);
		shader->setVec4("uTint", settings.tint);
		shader->setFloat("uMetallic", glm::clamp(settings.metallic, 0.0f, 1.0f));
		shader->setFloat("uRoughness", glm::clamp(settings.roughness, 0.04f, 1.0f));
		shader->setBool("uFullBrightOverride", settings.fullBrightOverride);
		shader->setVec3("uAmbientColor", glm::max(settings.ambientColor, glm::vec3(0.0f)));
		shader->setFloat("uAmbientIntensity", std::max(0.0f, settings.ambientIntensity));
		shader->setVec3("uCameraPos", settings.cameraWorldPosition);

		const std::vector<ModelLightData>* activeLights = settings.lights;
		const int lightCount = (activeLights == nullptr) ? 0 : static_cast<int>(std::min(activeLights->size(), static_cast<size_t>(MODEL_MAX_LIGHTS)));
		shader->setInt("uLightCount", lightCount);
		for (int i = 0; i < lightCount; ++i) {
			const ModelLightData& light = (*activeLights)[static_cast<size_t>(i)];
			const std::string prefix = "uLights[" + std::to_string(i) + "]";
			shader->setInt(prefix + ".type", light.type);
			shader->setVec3(prefix + ".position", light.position);
			shader->setVec3(prefix + ".direction", light.direction);
			shader->setVec3(prefix + ".color", light.color);
			shader->setFloat(prefix + ".intensity", light.intensity);
			shader->setFloat(prefix + ".range", light.range);
		}

		// render
		glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);
		for (const Submesh& submesh : submeshes) {
			if (!submesh.mesh || submesh.mesh->geometry.indices.empty()) {
				continue;
			}
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, submesh.textureID > 0 ? submesh.textureID : getFallbackTexture());
			shader->setInt("textureSRC", 0);
			glBindVertexArray(submesh.mesh->VAO);
			glDrawElements(GL_TRIANGLES, submesh.mesh->geometry.indices.size(), GL_UNSIGNED_INT, 0);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void destroy() {
		clearSubmeshes();
		if (shader) {
			delete shader;
			shader = nullptr;
		}
	}

private:
	static std::string resolveExistingPath(std::initializer_list<const char*> candidates) {
		for (const char* candidate : candidates) {
			std::ifstream file(candidate);
			if (file.good()) {
				return std::string(candidate);
			}
		}
		return std::string(*candidates.begin());
	}

	void setSingleMesh(const Geometry& geometry, unsigned int meshTextureID) {
		clearSubmeshes();
		Submesh submesh;
		submesh.mesh = new Mesh(geometry);
		submesh.textureID = meshTextureID > 0 ? meshTextureID : getFallbackTexture();
		submeshes.push_back(submesh);
		mesh = submeshes[0].mesh;
		textureID = submeshes[0].textureID;
	}

	void clearSubmeshes() {
		for (Submesh& submesh : submeshes) {
			if (submesh.mesh) {
				glDeleteVertexArrays(1, &submesh.mesh->VAO);
				glDeleteBuffers(1, &submesh.mesh->VBO);
				glDeleteBuffers(1, &submesh.mesh->EBO);
				delete submesh.mesh;
				submesh.mesh = nullptr;
			}
		}
		submeshes.clear();
		mesh = nullptr;
	}

	static unsigned int getFallbackTexture() {
		static unsigned int fallbackTextureID = 0;
		if (fallbackTextureID != 0) {
			return fallbackTextureID;
		}
		unsigned char whitePixel[] = { 255, 255, 255, 255 };
		glGenTextures(1, &fallbackTextureID);
		glBindTexture(GL_TEXTURE_2D, fallbackTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
		glBindTexture(GL_TEXTURE_2D, 0);
		return fallbackTextureID;
	}
};

#endif /* MODEL_H */
