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

struct MaterialComponent {
	glm::vec4 tint = glm::vec4(1.0f);
	float metallic = 0.0f;
	float roughness = 1.0f;
	bool wireframe = false;
};

struct RenderableComponent {
	Model* model = nullptr;
};

struct Entity {
	std::string name = "Entity";
	int order = 0;
	Transform transform;
	Entity* parent = nullptr;
	std::vector<Entity*> children;
	bool hasRenderable = false;
	RenderableComponent renderable;
	MaterialComponent material;
	bool showOriginMarker = true;

	Entity() = default;
};

struct Scene {
	std::string name;
	glm::vec4 color;
	std::vector<Entity*> entities;
	bool wireframeMode = false;
	Transform offset;

	Scene(std::string name = "scene", glm::vec4 color = glm::vec4(1)) {
		this->name = name;
		this->color = color;
	}

	void add(Model* model) {
		if (model == nullptr) {
			return;
		}
		Entity* entity = new Entity();
		entity->name = model->name ? model->name : "Entity";
		entity->order = model->order;
		entity->transform = model->transform;
		entity->hasRenderable = true;
		entity->renderable.model = model;
		entities.push_back(entity);
	}

	void addEntity(Entity* entity, Entity* parent = nullptr) {
		if (entity == nullptr) {
			return;
		}
		entity->parent = parent;
		if (parent != nullptr) {
			parent->children.push_back(entity);
		} else {
			entities.push_back(entity);
		}
	}

	Entity* createEmptyEntity(const std::string& entityName, const Transform& transform = Transform(), Entity* parent = nullptr) {
		Entity* entity = new Entity();
		entity->name = entityName;
		entity->transform = transform;
		entity->hasRenderable = false;
		entity->showOriginMarker = true;
		addEntity(entity, parent);
		return entity;
	}

	int renderableCount() const {
		int count = 0;
		for (Entity* entity : entities) {
			count += renderableCountRecursive(entity);
		}
		return count;
	}

	bool removeEntity(Entity* target) {
		if (target == nullptr) {
			return false;
		}
		if (target->parent != nullptr) {
			auto& siblings = target->parent->children;
			auto it = std::find(siblings.begin(), siblings.end(), target);
			if (it != siblings.end()) {
				siblings.erase(it);
				destroyEntityRecursive(target);
				return true;
			}
			return false;
		}
		auto it = std::find(entities.begin(), entities.end(), target);
		if (it != entities.end()) {
			entities.erase(it);
			destroyEntityRecursive(target);
			return true;
		}
		return false;
	}

	void update(double delta) {
		for (int i = 0; i < entities.size(); i++) {
			updateEntityRecursive(entities.at(i), delta);
		}
	}

	void clear() {
		for (int i = 0; i < entities.size(); i++) {
			destroyEntityRecursive(entities.at(i));
			entities.at(i) = nullptr;
		}
		entities.clear();
	}

	void destroy() {
		clear();
	}

private:
	static int renderableCountRecursive(const Entity* entity) {
		if (entity == nullptr) {
			return 0;
		}
		int count = (entity->hasRenderable && entity->renderable.model != nullptr) ? 1 : 0;
		for (const Entity* child : entity->children) {
			count += renderableCountRecursive(child);
		}
		return count;
	}

	static void updateEntityRecursive(Entity* entity, double delta) {
		if (entity == nullptr) {
			return;
		}
		if (entity->hasRenderable && entity->renderable.model != nullptr) {
			entity->renderable.model->transform = entity->transform;
			entity->renderable.model->order = entity->order;
			entity->renderable.model->update(delta);
		}
		for (Entity* child : entity->children) {
			updateEntityRecursive(child, delta);
		}
	}

	static void destroyEntityRecursive(Entity* entity) {
		if (entity == nullptr) {
			return;
		}
		for (Entity* child : entity->children) {
			destroyEntityRecursive(child);
		}
		entity->children.clear();
		if (entity->hasRenderable && entity->renderable.model != nullptr) {
			entity->renderable.model->destroy();
			delete entity->renderable.model;
			entity->renderable.model = nullptr;
		}
		delete entity;
	}
};

#endif
