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
 * \brief  Simple demo implementation of the Game class.
 *
 *
 * \author Lincoln Scheer
 * \date   March 2022
 *********************************************************************/

#include "Game.h"
#include <filesystem>

using namespace Nelson;
namespace fs = std::filesystem;

class MyGame : public Game{
public:
	std::vector<Model*> loadedModels;
	std::vector<std::string> loadedModelNames;

	void start() override {
		scene = new Scene("example-scene", glm::vec4(0.25, 0.25, 0.35, 1.0));
		const std::vector<fs::path> modelRoots = {
			fs::path("res/models"),
			fs::path("../res/models")
		};

		bool foundModelRoot = false;
		for (const fs::path& root : modelRoots) {
			if (!fs::exists(root) || !fs::is_directory(root)) {
				continue;
			}
			foundModelRoot = true;
			loadModelsRecursively(root);
			break;
		}

		if (!foundModelRoot) {
			std::cout << "[Demo] No model root found at 'res/models'." << std::endl;
		}
		if (loadedModels.empty()) {
			std::cout << "[Demo] No OBJ models were loaded." << std::endl;
		} else {
			std::cout << "[Demo] Loaded " << loadedModels.size() << " model(s) from nested folders." << std::endl;
		}
	}

	void update() override{
		// Static demo scene.
	}

	void dispose() override{ 
		// Called before engine shuts down...
	}

private:
	void loadModelsRecursively(const fs::path& root) {
		for (const auto& entry : fs::recursive_directory_iterator(root)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			const fs::path& file = entry.path();
			std::string extension = file.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			if (extension != ".obj") {
				continue;
			}

			loadedModelNames.push_back(file.stem().string());
			Model* model = new Model(loadedModelNames.back().c_str(), Transform());
			const std::string objPath = file.generic_string();
			const std::string mtlDir = file.parent_path().generic_string() + "/";
			if (model->LoadOBJ(objPath.c_str(), mtlDir.c_str())) {
				model->transform.scale = glm::vec3(0.01f);
				const float xOffset = static_cast<float>(loadedModels.size()) * 2.5f;
				model->transform.position = glm::vec3(xOffset, 0.0f, -2.0f);
				this->scene->add(model);
				loadedModels.push_back(model);
				std::cout << "[Demo] Loaded OBJ: " << objPath << std::endl;
			} else {
				std::cout << "[Demo] Failed OBJ: " << objPath << std::endl;
				delete model;
			}
		}
	}
};

int main() {
	MyGame game;
	game.run();
}
