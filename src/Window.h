//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Window.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  02-03-2022
// 
// PURPOSE:  A class that serves as a GLFWwindow handle and provides several
// functions for interacting with the window and its input.
// 
//---------------------------------------------------------------------------//

#pragma once

#include "Nelson.h"

class Window : System {
public:
	Window(MessageBus* bus)		// Attach System to Message Bus
	: System({ENGINE_EVENT, WINDOW_EVENT, INPUT_EVENT},bus) {} 

	void init();			// Create window handle

	bool isOpen();			// Returns true if window doesn't want
					// to close

	void update() override;		// Called at the end of every frame

	void dispose();		// Dispose window handle

	GLFWwindow* getWindow();	// Return window handle

	GLFWmonitor* getMonitor();	// Return monitor handle

	const GLFWvidmode* getVidMode();	// Return video mode handle

	// Avoid using this function because we now rely on the message bus for
	// recieving input.
	bool isKeyDown(int key);	// Return true if key pressed

private:
	// Handles
	GLFWmonitor* monitor	= nullptr;
	const GLFWvidmode* mode = nullptr;
	GLFWwindow* window	= nullptr;

	// System Callback
	void onNotify(Message message);

	// GLFW Member Create Functions
	void createMonitor();		// Returns secondary monitor if availible for debugging,
					// returns main window if not.

	void createMode();		// Create video mode based on the monitor

	void createWindow(bool fullscreen);	// Creates window with size depending on
						// the input paramater fullscreen.

	// Handle Callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};