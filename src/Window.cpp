/******************************************************************************
 *
 * @file	Window.cpp
 *
 * @author	Lincoln Scheer
 * @since	02/03/2022
 *
 * @brief	Window is a Nelson utility that provides a high-level
 *		interface for creating and manipulating a cross platform window
 *		that holds an opengl context.
 *
 *****************************************************************************/

#include "Window.h"

Window::Window(const char* title) {
        this->title = title;
        this->monitor = nullptr;
        this->mode = nullptr;
        this->window = nullptr;
}

void Window::init() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        createMonitor();
        createMode();
        createWindow(true);
        if (GLEW_OK != glewInit()) {
                glfwTerminate();
        }     
        glfwSwapInterval(1);
}

void Window::show() {
        glfwShowWindow(window);
}

bool Window::isOpen() {
        return !glfwWindowShouldClose(window);
}

void Window::hide() {
        glfwHideWindow(window);
}

void Window::update() {
        glfwSwapBuffers(window);
        glfwPollEvents();
}

void Window::terminate() {
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
                window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
        } else if(!fullscreen) {
                window = glfwCreateWindow(mode->width, mode->height, title, NULL, NULL);
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
        // tell GLFW to capture our mouse
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, (height - width) / 2, width, width);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
        }
}

void Window::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}