#ifndef ENGINE_H
#define ENGINE_H

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class Engine {
public:
    // Constructor
    Engine() { Init(); }

    // Destructor
    ~Engine() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    int width { 640 };
    int height { 480 };
    std::string windowName;
    GLFWwindow* window { nullptr };

    void Init() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }
};

#endif
