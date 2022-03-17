/*****************************************************************//**
 * \file   Game.cpp
 * \brief  Abstracts boilerplate code required for running a game and
 * editor.
 *
 * Class will be abstracted to create a runnable game.
 *
 * \author Lincoln Scheer
 * \date   March 2022
 *********************************************************************/

#include "Game.h"

Nelson::Game::Game() {
	this->console = new Console(&bus);
	this->window = new Window(&bus);
	this->renderer = new Renderer(&bus);
	this->editor = new Editor(&bus);
}

void Nelson::Game::_init() {
	window->init();
	renderer->init();
	editor->init(window->getWindow());
}

void Nelson::Game::_dispose() {
	if (scene == nullptr) {
		scene->destroy();
	}
	editor->terminate();
	window->dispose();
}

void Nelson::Game::run() {
	_init();
	start();
	if (scene != nullptr) {
		_loop();
		dispose();
		_dispose();
	}
	else {
		bus.sendMessage(Message({ ENGINE_EVENT }, "ENGINE_NO_SCENE"));
		bus.sendMessage(Message({ ENGINE_EVENT }, "ENGINE_EXIT"));
	}
}

void Nelson::Game::sendMessage(Message message) {
	bus.sendMessage(message);
}

void Nelson::Game::_loop() {
	float lastFrame = 0.0f;

	glm::vec2 mouseLast = glm::vec2(0);
	glm::vec2 mouseCurr = glm::vec2(0);
	glm::vec2 mouseDelta = glm::vec2(0);
	double dragSpeed = 0.05f;
	double zoomSpeed = 1.0f;
	bool resetMouse = false;

	while (window->isOpen()) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Mouse Drag
		if (glfwGetKey(window->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			mouseLast = mouseCurr;
			double x, y;
			glfwGetCursorPos(window->getWindow(), &x, &y);
			mouseCurr = glm::vec2(x, y);
			mouseDelta = mouseLast - mouseCurr;
			scene->offset.position.x -= mouseDelta.x * dragSpeed * deltaTime;
			scene->offset.position.y += mouseDelta.y * dragSpeed * deltaTime;
		}
		double x, y;
		glfwGetCursorPos(window->getWindow(), &x, &y);
		mouseCurr = glm::vec2(x, y);
		// Mouse Zoom
		if (glfwGetKey(window->getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS && glfwGetKey(window->getWindow(), GLFW_KEY_EQUAL) == GLFW_PRESS) {
			scene->offset.scale.x += zoomSpeed * deltaTime;
			scene->offset.scale.y += zoomSpeed * deltaTime;
		}
		else if (glfwGetKey(window->getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS && glfwGetKey(window->getWindow(), GLFW_KEY_MINUS) == GLFW_PRESS) {
			scene->offset.scale.x -= zoomSpeed * deltaTime;
			scene->offset.scale.y -= zoomSpeed * deltaTime;
		}

		update();

		scene->update(deltaTime);

		renderer->render(scene);

		editor->draw(*scene);

		window->update();
		bus.notify();
	}

}