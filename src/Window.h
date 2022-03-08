/******************************************************************************
 *
 * @file	Window.h
 *
 * @author	Lincoln Scheer
 * @since	02/03/2022
 *
 * @brief	Window is a Nelson utility that provides a high-level
 *		interface for creating and manipulating a cross platform window
 *		that holds an opengl context.
 *
 *****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include "NelsonEngine.h"

class Window {
public:
	/**
	 * Sets member title variable.
	 *
	 * \param title
	 */
	Window(const char* title);

	/**
	 * Creates fullscreen window on primary monitor.
	 *
	 */
	void init();

	/**
	 * Display window if hidden.
	 *
	 */
	void show();

	/**
	 * Hide window if displayed.
	 *
	 */
	void hide();

	/**
	 * Returns the inverse of glfwWindowShouldClose() withought having to
	 * pass the GLFWwindow handle.
	 *
	 * \return
	 */
	bool isOpen();

	/**
	 * Swaps renderable buffers and polls window for input events.
	 *
	 */
	void update();

	/**
	 * Frees all GLFW allocations made.
	 *
	 */
	void terminate();

	/**
	 * Return member GLFWwindow handle.
	 *
	 * \return
	 */
	GLFWwindow* getWindow();

	/**
	 * Return member GLFWmonitor handle.
	 *
	 * \return
	 */
	GLFWmonitor* getMonitor();

	/**
	 * Return member const GLFWvidmode.
	 *
	 * \return
	 */
	const GLFWvidmode* getVidMode();

	/**
	 * Return true if key is pressed.
	 * 
	 * \param key
	 * \return 
	 */
	bool isKeyDown(int key);

private:

	// Window Title, Used For Wincow Creation
	const char* title;

	// GLFW Members
	GLFWmonitor* monitor;
	const GLFWvidmode* mode;
	GLFWwindow* window;

	// GLFW Member Create Functions
	void createMonitor(); // Returns secondary monitor if availible for debugging, returns main window if not.
	void createMode();
	void createWindow(bool fullscreen);

	// Static Callback Functions
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif // WINDOW_H