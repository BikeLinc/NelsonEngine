/*****************************************************************//**
 *  _   _      _                       _____             _
 * | \ | | ___| |___  ___  _ __       | ____|_ __   __ _(_)_ __   ___
 * |  \| |/ _ \ / __|/ _ \| '_ \ _____|  _| | '_ \ / _` | | '_ \ / _ \
 * | |\  |  __/ \__ \ (_) | | | |_____| |___| | | | (_| | | | | |  __/
 * |_| \_|\___|_|___/\___/|_| |_|     |_____|_| |_|\__, |_|_| |_|\___|
 *                                                 |___/
 *
 * NelsonEngine v1.0 - Copyright(c) 2022 Lincoln Scheer
 *
 * Nelson Engine is a 2D and 3D game engine licensed under the MIT
 * License. Please see https://github.com/bikelinc/nelsonengine for
 * development notes and more information.
 *
 * \file   main.cpp
 * \brief  Runtime bootstrap that loads scene JSON and starts the game loop.
 *
 * \author Lincoln Scheer
 * \date   March 2022
 *********************************************************************/

#include "Game.h"
#include "SimpleJson.h"

#include <filesystem>

using namespace Nelson;
namespace fs = std::filesystem;

class MyGame : public Game{
public:
	explicit MyGame(std::string startupScenePath = "")
		: startupScenePath(std::move(startupScenePath)) {}

	void start() override {
		scene = new Scene("runtime-scene", glm::vec4(0.25f, 0.25f, 0.35f, 1.0f));
		sendMessage(Message({ CONSOLE_EVENT }, "[Scene] Runtime boot."));

		std::vector<std::string> sceneCandidates;
		if (!startupScenePath.empty()) {
			sceneCandidates.push_back(startupScenePath);
		}
		sceneCandidates.push_back("projects/default/scenes/default.scene.json");
		sceneCandidates.push_back("../projects/default/scenes/default.scene.json");
		sceneCandidates.push_back("res/scenes/default.scene.json");
		sceneCandidates.push_back("../res/scenes/default.scene.json");

		bool loaded = false;
		std::string loadedPath;
		std::string lastError;
		for (const std::string& scenePath : sceneCandidates) {
			if (!fs::exists(scenePath)) {
				continue;
			}

			if (loadSceneJson(scenePath, *scene, &lastError)) {
				loaded = true;
				loadedPath = scenePath;
				break;
			}
		}

			if (loaded) {
				sendMessage(Message({ CONSOLE_EVENT }, "[Scene] Loaded JSON scene: " + loadedPath));
				sendMessage(Message({ CONSOLE_EVENT }, "[Scene] Entities: " + std::to_string(scene->entities.size())));
			} else {
			if (!lastError.empty()) {
				sendMessage(Message({ CONSOLE_EVENT }, "[Scene] Failed to load startup scene: " + lastError));
			} else {
				sendMessage(Message({ CONSOLE_EVENT }, "[Scene] No startup JSON scene found."));
			}
		}
	}

	void update() override{
		// Runtime update hook.
	}

	void dispose() override{
		// Runtime dispose hook.
	}

private:
	std::string startupScenePath;

	static bool jsonToVec2(const SimpleJson::Value* value, glm::vec2& out) {
		if (value == nullptr || !value->isArray() || value->arrayValue.size() != 2) {
			return false;
		}
		if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber()) {
			return false;
		}
		out.x = static_cast<float>(value->arrayValue[0].numberValue);
		out.y = static_cast<float>(value->arrayValue[1].numberValue);
		return true;
	}

	static bool jsonToVec3(const SimpleJson::Value* value, glm::vec3& out) {
		if (value == nullptr || !value->isArray() || value->arrayValue.size() != 3) {
			return false;
		}
		if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber() || !value->arrayValue[2].isNumber()) {
			return false;
		}
		out.x = static_cast<float>(value->arrayValue[0].numberValue);
		out.y = static_cast<float>(value->arrayValue[1].numberValue);
		out.z = static_cast<float>(value->arrayValue[2].numberValue);
		return true;
	}

	static bool jsonToVec4(const SimpleJson::Value* value, glm::vec4& out) {
		if (value == nullptr || !value->isArray() || value->arrayValue.size() != 4) {
			return false;
		}
		if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber() ||
			!value->arrayValue[2].isNumber() || !value->arrayValue[3].isNumber()) {
			return false;
		}
		out.x = static_cast<float>(value->arrayValue[0].numberValue);
		out.y = static_cast<float>(value->arrayValue[1].numberValue);
		out.z = static_cast<float>(value->arrayValue[2].numberValue);
		out.w = static_cast<float>(value->arrayValue[3].numberValue);
		return true;
	}

	static std::string firstExistingPath(const std::string& rawPath, const fs::path& sceneFileDir, bool wantDirectory) {
		std::vector<fs::path> candidates;
		candidates.push_back(fs::path(rawPath));

		if (!rawPath.empty() && rawPath[0] != '/' && !(rawPath.size() > 1 && rawPath[1] == ':')) {
			candidates.push_back((sceneFileDir / rawPath).lexically_normal());
			candidates.push_back((fs::path("..") / rawPath).lexically_normal());
		}

		for (const fs::path& p : candidates) {
			if (wantDirectory) {
				if (fs::exists(p) && fs::is_directory(p)) {
					std::string resolved = p.generic_string();
					if (!resolved.empty() && resolved.back() != '/') {
						resolved.push_back('/');
					}
					return resolved;
				}
			} else {
				if (fs::exists(p) && fs::is_regular_file(p)) {
					return p.generic_string();
				}
			}
		}

		return rawPath;
	}

	bool loadSceneJson(const std::string& path, Scene& sceneRef, std::string* error) {
		SimpleJson::Value root;
		if (!SimpleJson::parseFile(path, root, error)) {
			return false;
		}

		const SimpleJson::Value* sceneObj = root.get("scene");
		if (sceneObj == nullptr || !sceneObj->isObject()) {
			if (error != nullptr) {
				*error = "Missing 'scene' object.";
			}
			return false;
		}

		glm::vec4 loadedColor;
		if (!jsonToVec4(sceneObj->get("color"), loadedColor)) {
			if (error != nullptr) {
				*error = "Invalid scene color.";
			}
			return false;
		}

		const SimpleJson::Value* modelsValue = sceneObj->get("models");
		if (modelsValue == nullptr || !modelsValue->isArray()) {
			if (error != nullptr) {
				*error = "Missing models array.";
			}
			return false;
		}

			sceneRef.clear();
			const SimpleJson::Value* nameValue = sceneObj->get("name");
			sceneRef.name = (nameValue != nullptr && nameValue->isString()) ? nameValue->stringValue : "scene";
			sceneRef.color = loadedColor;

			fs::path sceneFileDir = fs::path(path).parent_path();
			std::function<Entity*(const SimpleJson::Value&, Entity*)> loadEntityNode =
				[&](const SimpleJson::Value& modelValue, Entity* parent) -> Entity* {
					if (!modelValue.isObject()) {
						return nullptr;
					}
					const SimpleJson::Value* modelName = modelValue.get("name");
					const SimpleJson::Value* modelType = modelValue.get("type");
					const SimpleJson::Value* modelOrder = modelValue.get("order");
					const SimpleJson::Value* modelTransform = modelValue.get("transform");
					if (modelName == nullptr || !modelName->isString() ||
						modelType == nullptr || !modelType->isString() ||
						modelOrder == nullptr || !modelOrder->isNumber() ||
						modelTransform == nullptr || !modelTransform->isObject()) {
						return nullptr;
					}

					Transform transform;
					if (!jsonToVec3(modelTransform->get("position"), transform.position) ||
						!jsonToVec3(modelTransform->get("rotation"), transform.rotation) ||
						!jsonToVec3(modelTransform->get("scale"), transform.scale)) {
						return nullptr;
					}

					Entity* entity = new Entity();
					entity->name = modelName->stringValue;
					entity->order = static_cast<int>(modelOrder->numberValue);
					entity->transform = transform;
					entity->hasRenderable = false;
					entity->parent = parent;

					const SimpleJson::Value* hasRenderable = modelValue.get("has_renderable");
					if (hasRenderable != nullptr && hasRenderable->isBool()) {
						entity->hasRenderable = hasRenderable->boolValue;
					}
					const SimpleJson::Value* showOriginMarker = modelValue.get("show_origin_marker");
					if (showOriginMarker != nullptr && showOriginMarker->isBool()) {
						entity->showOriginMarker = showOriginMarker->boolValue;
					} else {
						entity->showOriginMarker = !entity->hasRenderable;
					}
					const SimpleJson::Value* materialObj = modelValue.get("material");
					if (materialObj != nullptr && materialObj->isObject()) {
						jsonToVec4(materialObj->get("tint"), entity->material.tint);
						const SimpleJson::Value* metallic = materialObj->get("metallic");
						if (metallic != nullptr && metallic->isNumber()) {
							entity->material.metallic = static_cast<float>(metallic->numberValue);
						}
						const SimpleJson::Value* roughness = materialObj->get("roughness");
						if (roughness != nullptr && roughness->isNumber()) {
							entity->material.roughness = static_cast<float>(roughness->numberValue);
						}
						const SimpleJson::Value* wireframe = materialObj->get("wireframe");
						if (wireframe != nullptr && wireframe->isBool()) {
							entity->material.wireframe = wireframe->boolValue;
						}
					}

					Model* model = nullptr;
					if (modelType->stringValue == "obj") {
						const SimpleJson::Value* objPath = modelValue.get("obj_path");
						const SimpleJson::Value* mtlDir = modelValue.get("mtl_dir");
						if (objPath == nullptr || !objPath->isString() || mtlDir == nullptr || !mtlDir->isString()) {
							delete entity;
							return nullptr;
						}
						std::string resolvedObjPath = firstExistingPath(objPath->stringValue, sceneFileDir, false);
						std::string resolvedMtlDir = firstExistingPath(mtlDir->stringValue, sceneFileDir, true);
						model = new Model(modelName->stringValue.c_str(), transform);
						if (!model->LoadOBJ(resolvedObjPath.c_str(), resolvedMtlDir.c_str())) {
							delete model;
							delete entity;
							return nullptr;
						}
					} else if (modelType->stringValue == "plane") {
						const SimpleJson::Value* texture = modelValue.get("texture");
						const SimpleJson::Value* size = modelValue.get("size");
						glm::vec2 bounds;
						if (texture == nullptr || !texture->isString() || !jsonToVec2(size, bounds)) {
							delete entity;
							return nullptr;
						}
						const std::string resolvedTexture = firstExistingPath(texture->stringValue, sceneFileDir, false);
						model = new Model(modelName->stringValue.c_str(), resolvedTexture.c_str(), bounds, transform);
					} else if (modelType->stringValue == "empty") {
						model = nullptr;
					} else {
						delete entity;
						return nullptr;
					}

					if (model != nullptr) {
						model->order = entity->order;
						model->transform = entity->transform;
						entity->renderable.model = model;
						entity->hasRenderable = true;
					}

					const SimpleJson::Value* childrenValue = modelValue.get("children");
					if (childrenValue != nullptr && childrenValue->isArray()) {
						for (const SimpleJson::Value& childValue : childrenValue->arrayValue) {
							Entity* child = loadEntityNode(childValue, entity);
							if (child != nullptr) {
								entity->children.push_back(child);
							}
						}
					}
					return entity;
				};

			for (const SimpleJson::Value& modelValue : modelsValue->arrayValue) {
				Entity* rootEntity = loadEntityNode(modelValue, nullptr);
				if (rootEntity != nullptr) {
					sceneRef.addEntity(rootEntity);
				}
			}

		return true;
	}
};

int main(int argc, char** argv) {
	std::string startupScenePath;
	if (argc > 1 && argv[1] != nullptr) {
		startupScenePath = argv[1];
	}

	MyGame game(startupScenePath);
	game.run();
}
