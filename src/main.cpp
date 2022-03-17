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

using namespace Nelson;

class MyGame : public Game{
public:

	Model * cube = nullptr;

	void start() override {
		scene = new Scene("example-scene", glm::vec4(0.25, 0.25, 0.35, 1.0));
		cube = new Model("Cube", "../res/images/blocks.jpg", CubeGeometry(), Transform());
		cube->transform.scale = glm::vec3(0.5f);
		this->scene->add(cube);
	}

	void update() override{
		cube->transform.rotation.y += 10.0f * deltaTime;
		cube->transform.rotation.z += 10.0f * deltaTime;
	}

	void dispose() override{ 
		// Called before engine shuts down...
	}
};

int main() {
	MyGame game;
	game.run();
}