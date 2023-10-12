#ifndef APP_H
#define APP_H

#include "engine_window.h"
#include "engine_pipeline.h"
#include "engine_swap_chain.h"
#include "engine_device.h"

#include <iostream>

namespace Engine{
class Application
{
public:
    static constexpr int WIDTH = 640;
    static constexpr int HEIGHT = 400;

    void run()
    {
        while (!engineWindow.shouldClose()) {
            glfwPollEvents();
        }
    }

private:

    EngineWindow engineWindow {WIDTH, HEIGHT, "VulkanEngine"};
    EngineDevice engineDevice{engineWindow};
    EngineSwapChain engineSwapChain{engineDevice, engineWindow.getExtent()};
    EnginePipeline enginePipeline {
        engineDevice, 
        EnginePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT), 
        "shaders/shader.vert.spv", 
        "shaders/shader.frag.spv"};
};
}

#endif