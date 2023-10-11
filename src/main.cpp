#include "engine_window.h"
#include <iostream>

int main() {
    EngineWindow engineWindow {640, 400, "VulkanEngine"};

    while (!engineWindow.shouldClose()) {
	    glfwPollEvents();
    }

    return 0;
}
