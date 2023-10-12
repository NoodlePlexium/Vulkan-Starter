#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <iostream>

class EngineWindow {

public:
    // Constructor
    EngineWindow(int _width, int _height, std::string _windowName) 
    : width(_width), height(_height), windowName(_windowName) 
    { Init(); }

    // Destructor
    ~EngineWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	EngineWindow(const EngineWindow &) = delete;
	EngineWindow &operator=(const EngineWindow &) = delete;

    bool shouldClose() {return glfwWindowShouldClose(window);}

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface){
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
			throw std::runtime_error("failed to create a window surface");
		}
	}

    VkExtent2D getExtent() {std::cout << "printing" <<std::endl; return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};}

	bool wasWindowResized() {return framebufferResized;}
	void resetWindowResizedFlag() {framebufferResized = false;}

	GLFWwindow *getGLFWwindow() const {return window;}

private:

    static void framebufferResizeCallBack(GLFWwindow *window, int width, int height){
		auto engineWindow = reinterpret_cast<EngineWindow *>(glfwGetWindowUserPointer(window));
		engineWindow->framebufferResized = true;
		engineWindow->width = width;
		engineWindow->height = height;
	}

	void Init(){
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallBack);
	}


	int width;
	int height;
	bool framebufferResized = false;

	std::string windowName;
	GLFWwindow *window;
};

#endif