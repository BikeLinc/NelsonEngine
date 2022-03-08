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

	Gui gui;
	gui.init(window.getWindow());

	Scene scene(glm::vec4(0.25, 0.25, 0.35, 1.0));

	TexturedPlane chong("Chong", "../res/images/chong.png", glm::vec2(0.25), Transform());
	TexturedPlane box("Box", "../res/images/rocks.jpg", glm::vec2(0.25), Transform());

	scene.add(&chong);
	scene.add(&box);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	while (window.isOpen()) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

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
