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
#include <thread>

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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        createMonitor();
        createMode();
        createWindow(false);
        if (GLEW_OK != glewInit()) {
                glfwTerminate();
        }     
        glfwSwapInterval(0);

        const char* monitorName = monitor ? glfwGetMonitorName(monitor) : "Unknown";
        const char* gpuVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* gpuRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        std::cout << "[Hardware] Monitor: " << (monitorName ? monitorName : "Unknown")
                  << " (" << mode->width << "x" << mode->height << " @ " << mode->refreshRate << "Hz)" << std::endl;
        std::cout << "[Hardware] Window: " << windowWidth << "x" << windowHeight << std::endl;
        std::cout << "[Hardware] GPU: " << (gpuVendor ? gpuVendor : "Unknown")
                  << " | " << (gpuRenderer ? gpuRenderer : "Unknown") << std::endl;
        std::cout << "[Hardware] OpenGL: " << (glVersion ? glVersion : "Unknown")
                  << " | GLSL: " << (glslVersion ? glslVersion : "Unknown") << std::endl;
        std::cout << "[Hardware] CPU threads: " << std::thread::hardware_concurrency() << std::endl;
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
        else if (event.rfind("KEY_", 0) == 0) {
                // Ignore unhandled key events to avoid console spam.
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
                const int windowWidth = std::max(1280, mode->width * 3 / 4);
                const int windowHeight = std::max(720, mode->height * 3 / 4);
                window = glfwCreateWindow(windowWidth, windowHeight, "Nelson", NULL, NULL);
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
        glViewport(0, 0, w, h);
        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
        // tell GLFW to capture our mouse
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
        Window* handler = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (action != GLFW_PRESS) {
                return;
        }

        if (key == GLFW_KEY_ESCAPE) {
                handler->postMessage(Message({ INPUT_EVENT }, "KEY_ESCAPE"));
        }
        else if (key == GLFW_KEY_SPACE) {
                handler->postMessage(Message({ INPUT_EVENT }, "KEY_SPACE"));
        }
        else {
                const char* keynameRaw = glfwGetKeyName(key, scancode);
                if (keynameRaw != nullptr) {
                        std::string keyname = keynameRaw;
                        std::transform(keyname.begin(), keyname.end(), keyname.begin(), ::toupper);
                        handler->postMessage(Message({ INPUT_EVENT }, "KEY_" + keyname));
                }
        }


}

void Window::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}
