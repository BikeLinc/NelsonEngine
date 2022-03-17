//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Engine.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  03-16-2022
// 
// PURPOSE:  API for manipulating Nelson Engine systems to do something
// meaningful.
// 
//---------------------------------------------------------------------------//

#include "Nelson.h"

Engine::Engine() {
	// Wake Up Engine... 
	// Instantiate Systems & Message Bus
	messageBus->sendMessage(Message({ ENGINE_EVENT }, "ENGINE_WAKE"));
}


void Engine::init() {
	// Initialize Systems
	window->init();
	renderer->init();
	editor->init(window->getWindow());
}


void Engine::start() {
	// If scene hasn't been loaded yet, load a demo one. The run() function is not
	// capable of running with scene == NULL;
	if (scene == nullptr) {
		scene = new Scene("demo-scene", glm::vec4(0.854, 0.243, 0.623, 1.0));
	}
}


int Engine::run() {
	// Start game loop
	if (scene == nullptr) {
		return -1;
	}

	while (!exit) {
		update();
		scene->update(deltaTime);
		renderer->render(scene);
		editor->drawInspector(*scene);
		window->update();
		messageBus->notify();
	}
	return 0;
}


void Engine::update() {
	sync();
	if (!window->isOpen()) {
		exit = true;
	}
}


void Engine::dispose() {
	if (scene != nullptr) {
		scene->destroy();
	}
	editor->terminate();
	window->terminate();
}


void Engine::sync() {
	// Use GLFW to get time since window opened and calculate time since
	// last call to sync().
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}