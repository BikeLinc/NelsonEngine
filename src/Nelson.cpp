#include "NelsonEngine.h"

class TexturedPlane : public Model {
public:
	TexturedPlane(const char* name, const char* texturePath, glm::vec2 meshBounds, Transform transformOrigin) : Model(name, texturePath, PlaneGeometry(meshBounds), transformOrigin) {

	}

	void update(double delta) override {
		// Implement Update Here
	}
};

int main() {
	Window window("Nelson");
	window.init();

	Renderer renderer;
	renderer.init();

	Editor gui;
	gui.init(window.getWindow());

	Scene scene(glm::vec4(0.25, 0.25, 0.35, 1.0));

	TexturedPlane chong("Chong", "../res/images/chong.png", glm::vec2(0.25), Transform());
	TexturedPlane box("Box", "../res/images/rocks.jpg", glm::vec2(0.25), Transform());

	scene.add(&chong);
	scene.add(&box);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	glm::vec2 mouseLast = glm::vec2(0);
	glm::vec2 mouseCurr = glm::vec2(0);
	glm::vec2 mouseDelta = glm::vec2(0);
	double dragSpeed = 0.05f;
	double zoomSpeed = 1.0f;
	bool resetMouse = false;

	while (window.isOpen()) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Mouse Drag
		if (glfwGetKey(window.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetMouseButton(window.getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			
			mouseLast = mouseCurr;
			double x, y;
			glfwGetCursorPos(window.getWindow(), &x, &y);
			mouseCurr = glm::vec2(x, y);
			mouseDelta = mouseLast - mouseCurr;
			scene.offset.position.x -= mouseDelta.x * dragSpeed * deltaTime;
			scene.offset.position.y += mouseDelta.y * dragSpeed * deltaTime;
		}
		double x, y;
		glfwGetCursorPos(window.getWindow(), &x, &y);
		mouseCurr = glm::vec2(x, y);

		// Mouse Zoom
		if (glfwGetKey(window.getWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS && glfwGetKey(window.getWindow(), GLFW_KEY_EQUAL) == GLFW_PRESS) {
			scene.offset.scale.x += zoomSpeed * deltaTime;
			scene.offset.scale.y += zoomSpeed * deltaTime;
		}
		else if (glfwGetKey(window.getWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS && glfwGetKey(window.getWindow(), GLFW_KEY_MINUS) == GLFW_PRESS) {
			scene.offset.scale.x -= zoomSpeed * deltaTime;
			scene.offset.scale.y -= zoomSpeed * deltaTime;
		}

		scene.update(deltaTime);

		renderer.render(&scene);

		gui.drawInspector(scene);

		window.update();
	}

	scene.destroy();

	gui.terminate();
	
	window.terminate();

	return 0;
}
