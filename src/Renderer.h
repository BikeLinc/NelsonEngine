/******************************************************************************
 *
 * @file	Renderer.h
 *
 * @author	Lincoln Scheer
 * @since	03/07/2022
 *
 * @brief	Renderer is a class that performs all drawing of renderables.
 *
 *****************************************************************************/

#ifndef RENDERER_H

#include "Nelson.h"

class Renderer : public System {
public:
	Renderer(MessageBus* bus) : System({ENGINE_EVENT, RENDER_EVENT }, bus) {}

	void init() {
		glEnable(GL_DEPTH_TEST);
	}

	void update() override {

	}

	void render(Scene* scene) {
		// Clear
		glClearColor(scene->color.x, scene->color.y, scene->color.z, scene->color.w);
		//glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Calculate Min and Max order
		int minOrder = 0, maxOrder = 0;
		for (int i = 0; i < scene->models.size(); i++) {
			int order = scene->models.at(i)->order;
			if (order < minOrder) {
				minOrder = order;
			}
			else if (order > maxOrder) {
				maxOrder = order;
			}
		}

		int orderIndex = minOrder;
		while (minOrder <= orderIndex && orderIndex <= maxOrder) {
			for (int i = 0; i < scene->models.size(); i++) {
				if (scene->models.at(i)->order == orderIndex) {
					scene->models.at(i)->draw(scene->offset);
					glClear(GL_DEPTH_BUFFER_BIT); // Must Clear Depth Buffer Every Time So Order MAtters
				}
			}
			orderIndex++;
		}
		
	}

	void onNotify(Message message) {
		postMessage(Message({ CONSOLE_EVENT }, "Message \'" + message.getEvent() + "\' is unhandled by Renderer"));
	}
};

#endif // !RENDERER_H
