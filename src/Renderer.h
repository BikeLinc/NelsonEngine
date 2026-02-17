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
	unsigned int markerVao = 0;
	unsigned int markerVbo = 0;
	unsigned int markerShaderProgram = 0;

	void init() {
		glEnable(GL_DEPTH_TEST);
		initMarkerRenderer();
	}

	void update() override {

	}

	void render(Scene* scene) {
		// Clear
		glClearColor(scene->color.x, scene->color.y, scene->color.z, scene->color.w);
		//glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, scene->wireframeMode ? GL_LINE : GL_FILL);

		std::vector<DrawItem> drawItems;
		drawItems.reserve(scene->entities.size() * 2);
		for (Entity* root : scene->entities) {
			collectDrawItems(root, Transform(), drawItems);
		}
		std::sort(drawItems.begin(), drawItems.end(), [](const DrawItem& a, const DrawItem& b) {
			const int orderA = (a.entity != nullptr) ? a.entity->order : 0;
			const int orderB = (b.entity != nullptr) ? b.entity->order : 0;
			return orderA < orderB;
		});

		for (const DrawItem& item : drawItems) {
			Entity* entity = item.entity;
			if (entity == nullptr) {
				continue;
			}

			if (entity->hasRenderable && entity->renderable.model != nullptr) {
				entity->renderable.model->transform = item.world;
				const bool entityWireframe = scene->wireframeMode || entity->material.wireframe;
				entity->renderable.model->draw(scene->offset, entity->material.tint, entityWireframe);
			} else if (entity->showOriginMarker) {
				drawOriginMarker(*scene, item.world, entity->material.tint);
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void onNotify(Message message) {
		postMessage(Message({ CONSOLE_EVENT }, "Message \'" + message.getEvent() + "\' is unhandled by Renderer"));
	}

private:
	struct DrawItem {
		Entity* entity = nullptr;
		Transform world;
	};

	static Transform combineTransforms(const Transform& parent, const Transform& local) {
		Transform world;
		world.position = parent.position + local.position;
		world.rotation = parent.rotation + local.rotation;
		world.scale = parent.scale * local.scale;
		return world;
	}

	static void collectDrawItems(Entity* entity, const Transform& parentWorld, std::vector<DrawItem>& out) {
		if (entity == nullptr) {
			return;
		}
		DrawItem item;
		item.entity = entity;
		item.world = combineTransforms(parentWorld, entity->transform);
		out.push_back(item);
		for (Entity* child : entity->children) {
			collectDrawItems(child, item.world, out);
		}
	}

	void initMarkerRenderer() {
		const char* markerVertexSource =
			"#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"uniform mat4 view;\n"
			"uniform mat4 projection;\n"
			"void main() {\n"
			"  gl_Position = projection * view * vec4(aPos, 1.0);\n"
			"  gl_PointSize = 8.0;\n"
			"}\n";

		const char* markerFragmentSource =
			"#version 330 core\n"
			"out vec4 FragColor;\n"
			"uniform vec4 markerColor;\n"
			"void main() {\n"
			"  vec2 uv = gl_PointCoord * 2.0 - 1.0;\n"
			"  if (dot(uv, uv) > 1.0) discard;\n"
			"  FragColor = markerColor;\n"
			"}\n";

		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &markerVertexSource, nullptr);
		glCompileShader(vertexShader);

		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &markerFragmentSource, nullptr);
		glCompileShader(fragmentShader);

		markerShaderProgram = glCreateProgram();
		glAttachShader(markerShaderProgram, vertexShader);
		glAttachShader(markerShaderProgram, fragmentShader);
		glLinkProgram(markerShaderProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glGenVertexArrays(1, &markerVao);
		glGenBuffers(1, &markerVbo);
		glBindVertexArray(markerVao);
		glBindBuffer(GL_ARRAY_BUFFER, markerVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	void drawOriginMarker(const Scene& scene, const Transform& worldTransform, const glm::vec4& color) {
		if (markerShaderProgram == 0 || markerVao == 0 || markerVbo == 0) {
			return;
		}

		const glm::vec3 position = worldTransform.position + scene.offset.position;
		const float markerVertex[3] = { position.x, position.y, position.z };
		glBindBuffer(GL_ARRAY_BUFFER, markerVbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(markerVertex), markerVertex);

		GLint viewport[4] = { 0, 0, 1, 1 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		float aspect = static_cast<float>(viewport[2]) / static_cast<float>(std::max(1, viewport[3]));
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 1.2f, 3.5f),
			glm::vec3(0.0f, 0.8f, -2.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.01f, 500.0f);

		glUseProgram(markerShaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(markerShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(markerShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform4fv(glGetUniformLocation(markerShaderProgram, "markerColor"), 1, glm::value_ptr(color));
		glBindVertexArray(markerVao);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
	}
};

#endif // !RENDERER_H
