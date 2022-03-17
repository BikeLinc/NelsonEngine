//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Window.cpp
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  02-03-2022
// 
// PURPOSE:  A class that serves as a GLFWwindow handle and provides several
// functions for interacting with the window and its input.
// 
//---------------------------------------------------------------------------//

#include "Window.h"

void Window::init() {
        this->monitor = nullptr;
        this->mode = nullptr;
        this->window = nullptr;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
// Must define forward compatability on osx because by default opengl is
// disabled but not yet depreceated.
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        createMonitor();
        createMode();
        createWindow(true);
        if (GLEW_OK != glewInit()) {
                glfwTerminate();
        }     
        glfwSwapInterval(0);
}

bool Window::isOpen() {
        return !glfwWindowShouldClose(window);
}

void Window::update() {
        glfwSwapBuffers(window);
        glfwPollEvents();
}

void Window::dispose () {
        glfwTerminate();
}

GLFWwindow* Window::getWindow() {
        return this->window;
}

GLFWmonitor* Window::getMonitor() {
        return this->monitor;
}

const GLFWvidmode* Window::getVidMode() {
        return this->mode;
}

bool Window::isKeyDown(int key) {
        return (glfwGetKey(window, key) == GLFW_PRESS);
}

void Window::onNotify(Message message) {
        std::string event = message.getEvent();
        if (event == "KEY_ESCAPE") {
                postMessage(Message({ ENGINE_EVENT }, "ENGINE_EXIT"));
        }
        else if (event == "ENGINE_EXIT") {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else {
                postMessage(Message({ CONSOLE_EVENT }, "Message \'" + event + "\' is unhandled by Window"));
        }
}

void Window::createMonitor() {
        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        monitor = monitors[count - 1];
}

void Window::createMode() {
        mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
}

void Window::createWindow(bool fullscreen) {
        if(fullscreen) {
                window = glfwCreateWindow(mode->width, mode->height, "Nelson", monitor, NULL);
        } else if(!fullscreen) {
                window = glfwCreateWindow(mode->width, mode->height, "Nelson", NULL, NULL);
        }
        if (window == NULL)
        {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetKeyCallback(window, &Window::key_callback);
        glfwSetCursorPosCallback(window, &Window::mouse_callback);
        glfwSetScrollCallback(window, &Window::scroll_callback);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, (h - w) / 2, w, w);
        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
        // tell GLFW to capture our mouse
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, (height - width) / 2, width, width);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
        Window* handler = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

        if (key == GLFW_KEY_ESCAPE) {
                handler->postMessage(Message({ CONSOLE_EVENT, INPUT_EVENT }, "KEY_ESCAPE"));
        }
        else if (key == GLFW_KEY_SPACE) {
                handler->postMessage(Message({ CONSOLE_EVENT, INPUT_EVENT }, "KEY_SPACE"));
        }
        else {
                std::string keyname = glfwGetKeyName(key, scancode);
                std::transform(keyname.begin(), keyname.end(), keyname.begin(), ::toupper);
                handler->postMessage(Message({ CONSOLE_EVENT, INPUT_EVENT }, "KEY_" + keyname));
        }


}

void Window::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}