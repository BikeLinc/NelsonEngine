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
#include <cmath>
#include <limits>

struct Entity;

struct MaterialComponent {
	glm::vec4 tint = glm::vec4(1.0f);
	float metallic = 0.0f;
	float roughness = 1.0f;
	bool wireframe = false;
};

enum class LightType : int {
	Directional = 0,
	Point = 1
};

struct LightComponent {
	bool enabled = true;
	LightType type = LightType::Point;
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 4.0f;
	float range = 16.0f;
};

struct CameraComponent {
	bool enabled = true;
	bool primary = false;
	float fovYDegrees = 60.0f;
	float nearClip = 0.01f;
	float farClip = 500.0f;
};

struct RenderableComponent {
	Model* model = nullptr;
};

struct SceneLightingSettings {
	bool fullBrightOverride = true;
	glm::vec3 ambientColor = glm::vec3(1.0f);
	float ambientIntensity = 1.0f;
};

struct SceneLight {
	int type = static_cast<int>(LightType::Point);
	glm::vec3 worldPosition = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float range = 1.0f;
};

struct SceneCamera {
	Entity* entity = nullptr;
	glm::vec3 worldPosition = glm::vec3(0.0f, 1.2f, 3.5f);
	glm::vec3 forward = glm::normalize(glm::vec3(0.0f, 0.8f, -2.0f) - glm::vec3(0.0f, 1.2f, 3.5f));
	float fovYDegrees = 60.0f;
	float nearClip = 0.01f;
	float farClip = 500.0f;
};

struct SceneRay {
	glm::vec3 origin = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
};

struct RaycastHit {
	Entity* entity = nullptr;
	float distance = std::numeric_limits<float>::max();
	glm::vec3 point = glm::vec3(0.0f);
};

constexpr int MAX_SCENE_LIGHTS = 8;

inline glm::vec3 DefaultCameraPosition() {
	return glm::vec3(0.0f, 1.2f, 3.5f);
}

inline glm::vec3 DefaultCameraTarget() {
	return glm::vec3(0.0f, 0.8f, -2.0f);
}

inline glm::mat4 DefaultSceneViewMatrix() {
	return glm::lookAt(DefaultCameraPosition(), DefaultCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));
}

inline glm::mat4 DefaultSceneProjectionMatrix(float aspect) {
	return glm::perspective(glm::radians(60.0f), aspect, 0.01f, 500.0f);
}

struct Entity {
	std::string name = "Entity";
	int order = 0;
	Transform transform;
	Entity* parent = nullptr;
	std::vector<Entity*> children;
	bool hasRenderable = false;
	RenderableComponent renderable;
	MaterialComponent material;
	bool hasLight = false;
	LightComponent light;
	bool hasCamera = false;
	CameraComponent camera;
	bool showOriginMarker = true;

	Entity() = default;
};

struct Scene {
	std::string name;
	glm::vec4 color;
	std::vector<Entity*> entities;
	bool wireframeMode = false;
	SceneLightingSettings lighting;
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

	int lightCount() const {
		int count = 0;
		for (Entity* entity : entities) {
			count += lightCountRecursive(entity);
		}
		return count;
	}

	int cameraCount() const {
		int count = 0;
		for (Entity* entity : entities) {
			count += cameraCountRecursive(entity);
		}
		return count;
	}

	bool getActiveCamera(SceneCamera& outCamera) const {
		bool foundAny = false;
		bool foundPrimary = false;
		SceneCamera anyCamera;
		SceneCamera primaryCamera;
		for (Entity* entity : entities) {
			collectCameraRecursive(entity, Transform(), foundAny, anyCamera, foundPrimary, primaryCamera);
			if (foundPrimary) {
				break;
			}
		}
		if (foundPrimary) {
			outCamera = primaryCamera;
			return true;
		}
		if (foundAny) {
			outCamera = anyCamera;
			return true;
		}
		outCamera.entity = nullptr;
		outCamera.worldPosition = DefaultCameraPosition();
		outCamera.forward = glm::normalize(DefaultCameraTarget() - DefaultCameraPosition());
		outCamera.fovYDegrees = 60.0f;
		outCamera.nearClip = 0.01f;
		outCamera.farClip = 500.0f;
		return false;
	}

	void getActiveCameraMatrices(float aspect, glm::mat4& outView, glm::mat4& outProjection, glm::vec3& outCameraPosition) const {
		SceneCamera camera;
		getActiveCamera(camera);
		const glm::vec3 target = camera.worldPosition + camera.forward;
		outView = glm::lookAt(camera.worldPosition, target, glm::vec3(0.0f, 1.0f, 0.0f));
		const float nearClip = std::max(0.001f, camera.nearClip);
		const float farClip = std::max(nearClip + 0.001f, camera.farClip);
		outProjection = glm::perspective(glm::radians(glm::clamp(camera.fovYDegrees, 5.0f, 175.0f)), aspect, nearClip, farClip);
		outCameraPosition = camera.worldPosition;
	}

	void collectActiveLights(std::vector<SceneLight>& out, size_t maxLights = MAX_SCENE_LIGHTS) const {
		out.clear();
		out.reserve(std::min(maxLights, static_cast<size_t>(MAX_SCENE_LIGHTS)));
		for (Entity* entity : entities) {
			collectActiveLightsRecursive(entity, Transform(), out, maxLights);
			if (out.size() >= maxLights) {
				break;
			}
		}
	}

	bool raycast(const SceneRay& ray, RaycastHit& outHit) const {
		outHit.entity = nullptr;
		outHit.distance = std::numeric_limits<float>::max();
		outHit.point = glm::vec3(0.0f);
		const glm::vec3 dir = glm::normalize(ray.direction);
		if (std::isnan(dir.x) || std::isnan(dir.y) || std::isnan(dir.z)) {
			return false;
		}

		for (Entity* entity : entities) {
			raycastRecursive(entity, Transform(), ray.origin, dir, outHit);
		}
		return outHit.entity != nullptr;
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
	static Transform combineTransforms(const Transform& parent, const Transform& local) {
		Transform world;
		world.position = parent.position + local.position;
		world.rotation = parent.rotation + local.rotation;
		world.scale = parent.scale * local.scale;
		return world;
	}

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

	static int lightCountRecursive(const Entity* entity) {
		if (entity == nullptr) {
			return 0;
		}
		int count = (entity->hasLight && entity->light.enabled) ? 1 : 0;
		for (const Entity* child : entity->children) {
			count += lightCountRecursive(child);
		}
		return count;
	}

	static int cameraCountRecursive(const Entity* entity) {
		if (entity == nullptr) {
			return 0;
		}
		int count = (entity->hasCamera && entity->camera.enabled) ? 1 : 0;
		for (const Entity* child : entity->children) {
			count += cameraCountRecursive(child);
		}
		return count;
	}

	static SceneCamera cameraFromTransform(Entity* entity, const Transform& world) {
		SceneCamera camera;
		camera.entity = entity;
		camera.worldPosition = world.position;
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		rotation = glm::rotate(rotation, glm::radians(world.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotation = glm::rotate(rotation, glm::radians(world.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotation = glm::rotate(rotation, glm::radians(world.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		forward = glm::normalize(glm::vec3(rotation * glm::vec4(forward, 0.0f)));
		camera.forward = forward;
		camera.fovYDegrees = entity->camera.fovYDegrees;
		camera.nearClip = entity->camera.nearClip;
		camera.farClip = entity->camera.farClip;
		return camera;
	}

	static void collectCameraRecursive(Entity* entity,
		const Transform& parentWorld,
		bool& foundAny,
		SceneCamera& anyCamera,
		bool& foundPrimary,
		SceneCamera& primaryCamera) {
		if (entity == nullptr || foundPrimary) {
			return;
		}
		const Transform world = combineTransforms(parentWorld, entity->transform);
		if (entity->hasCamera && entity->camera.enabled) {
			const SceneCamera camera = cameraFromTransform(entity, world);
			if (!foundAny) {
				foundAny = true;
				anyCamera = camera;
			}
			if (entity->camera.primary) {
				foundPrimary = true;
				primaryCamera = camera;
				return;
			}
		}
		for (Entity* child : entity->children) {
			collectCameraRecursive(child, world, foundAny, anyCamera, foundPrimary, primaryCamera);
			if (foundPrimary) {
				return;
			}
		}
	}

	static void collectActiveLightsRecursive(const Entity* entity, const Transform& parentWorld, std::vector<SceneLight>& out, size_t maxLights) {
		if (entity == nullptr || out.size() >= maxLights) {
			return;
		}
		const Transform world = combineTransforms(parentWorld, entity->transform);
		if (entity->hasLight && entity->light.enabled && out.size() < maxLights) {
			SceneLight light;
			light.type = static_cast<int>(entity->light.type);
			light.worldPosition = world.position;
			glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, 0.0f);
			glm::mat4 rotation = glm::mat4(1.0f);
			rotation = glm::rotate(rotation, glm::radians(world.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotation = glm::rotate(rotation, glm::radians(world.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotation = glm::rotate(rotation, glm::radians(world.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			lightDir = glm::vec3(rotation * glm::vec4(lightDir, 0.0f));
			light.direction = glm::normalize(lightDir);
			light.color = glm::max(entity->light.color, glm::vec3(0.0f));
			light.intensity = std::max(0.0f, entity->light.intensity);
			light.range = std::max(0.01f, entity->light.range);
			out.push_back(light);
		}
		for (const Entity* child : entity->children) {
			collectActiveLightsRecursive(child, world, out, maxLights);
			if (out.size() >= maxLights) {
				break;
			}
		}
	}

	static float approximateRenderableRadius(const Entity* entity) {
		if (entity == nullptr || !entity->hasRenderable || entity->renderable.model == nullptr || entity->renderable.model->mesh == nullptr) {
			return 0.0f;
		}
		const Model* model = entity->renderable.model;
		if (model->sourceType == "plane") {
			return std::max(model->sourcePlaneSize.x, model->sourcePlaneSize.y) * 0.75f;
		}
		const std::vector<float>& vertices = model->mesh->geometry.vertices;
		if (vertices.empty()) {
			return 1.0f;
		}
		const size_t stride = (vertices.size() % 8 == 0) ? 8 : 5;
		float maxLen = 0.0f;
		for (size_t i = 0; i + 2 < vertices.size(); i += stride) {
			const glm::vec3 p(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
			maxLen = std::max(maxLen, glm::length(p));
		}
		return std::max(0.1f, maxLen);
	}

	static bool raySphere(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& center, float radius, float& t) {
		const glm::vec3 oc = origin - center;
		const float a = glm::dot(direction, direction);
		const float b = 2.0f * glm::dot(oc, direction);
		const float c = glm::dot(oc, oc) - radius * radius;
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f) {
			return false;
		}
		const float sqrtD = std::sqrt(discriminant);
		const float inv = 0.5f / a;
		const float t0 = (-b - sqrtD) * inv;
		const float t1 = (-b + sqrtD) * inv;
		if (t0 > 0.0f) {
			t = t0;
			return true;
		}
		if (t1 > 0.0f) {
			t = t1;
			return true;
		}
		return false;
	}

	static void raycastRecursive(Entity* entity, const Transform& parentWorld, const glm::vec3& origin, const glm::vec3& direction, RaycastHit& bestHit) {
		if (entity == nullptr) {
			return;
		}
		const Transform world = combineTransforms(parentWorld, entity->transform);
		if (entity->hasRenderable && entity->renderable.model != nullptr) {
			const float radius = approximateRenderableRadius(entity) *
				std::max(std::max(std::abs(world.scale.x), std::abs(world.scale.y)), std::abs(world.scale.z));
			if (radius > 0.0f) {
				float t = 0.0f;
				if (raySphere(origin, direction, world.position, radius, t) && t < bestHit.distance) {
					bestHit.entity = entity;
					bestHit.distance = t;
					bestHit.point = origin + direction * t;
				}
			}
		}
		for (Entity* child : entity->children) {
			raycastRecursive(child, world, origin, direction, bestHit);
		}
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
