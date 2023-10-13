#ifndef APP_H
#define APP_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "engine_window.h"
#include "renderer.h"
#include "engine_device.h"
#include "engine_mesh.h"
#include "engine_game_object.h"
#include "engine_render_system.h"
#include "engine_point_light_system.h"
#include "engine_camera.h"
#include "engine_buffer.h"
#include "engine_descriptor.h"
#include "engine_input_system.h"

#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>

namespace Engine{

struct GlobalUbo {
	glm::mat4 projectionView{1.0f};
	glm::vec4 ambientLightColour{0.69f, 0.84f, 0.89f, 0.4f};
	glm::vec4 lightPosition {2.0f};
	alignas(16) glm::vec4 lightColour {1.0f};
	glm::mat4 view{1.0f};
};

class Application{
public:
	static constexpr int width = 800;
	static constexpr int height = 600;

	Application() {
		// Descriptor set pool
		globalPool = EngineDescriptorPool::Builder(engineDevice)
		.setMaxSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
		.build();
		loadGameObjects();
	}

	~Application() {}
	
	Application(const Application &) = delete;
	Application &operator=(const Application &) = delete;	


	void run() {

		// UNIFORM BUFFER OBJECT
		std::vector<std::unique_ptr<EngineBuffer>> uboBuffers(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i =0; i < uboBuffers.size(); ++i){
			uboBuffers[i] = std::make_unique<EngineBuffer>(
				engineDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		
			uboBuffers[i]->map();
		}

		// Descriptor sets
		auto globalSetLayout = EngineDescriptorSetLayout::Builder(engineDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); ++i){
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			EngineDescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
		}


	    // internal
	    float aspect = renderer.getAspectRatio();
	    auto currentTime = std::chrono::high_resolution_clock::now();
	    float frameTime;


	    // SCRIPTABLE ZONE //////////////////////////////////////////////////
	    Camera camera{};
		InputSystem input{window.getGLFWwindow()};
		input.SetMouseMode(MouseMode::Play);
	    camera.setPerspectiveProjection(aspect);

	    // RENDER SYSTEMS SETUP ///////////////////////////////
	    RenderSystem renderSystem{engineDevice, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()}; // Game Object Render System
		PointLightSystem pointLightSystem{engineDevice, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()}; // Point Light Render System
		
		// INTERNAL LOOP RUNS ONCE PER FRAME ///////////////////////////////
	    while (!window.shouldClose()) {
	        glfwPollEvents();

	        // calculates time elapsed since last frame
	        auto newTime = std::chrono::high_resolution_clock::now();
	        frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
	        currentTime = newTime;


			// Update input system state
    		input.UpdateInputs();

			float speed = 2.0f;

			glm::vec2 moveInput = input.Movement() * speed * frameTime;
			glm::vec2 mouseLook = input.MouseLook() * 0.00045f;
			glm::vec3 move = moveInput.x * camera.Right() + glm::vec3(0.0f, input.MovementY() * speed * frameTime, 0.0f) + moveInput.y * camera.Forward();
			glm::vec3 rot{mouseLook.y, -mouseLook.x, 0.0f};


			camera.position += move;
			camera.rotation += rot;
			camera.rotation.x = glm::clamp(camera.rotation.x, -glm::pi<float>() * 0.5f, glm::pi<float>() * 0.5f); // clamp

			// set camera view
			camera.setView();
			camera.setPerspectiveProjection(aspect);

			if (input.GetKeyDown(InputSystem::KeyCode::Escape))
			{
				if (input.GetMouseMode() == MouseMode::Play) input.SetMouseMode(MouseMode::Normal);
				else input.SetMouseMode(MouseMode::Play);
			} 


	        if (auto commandBuffer = renderer.beginFrame()) {
	        	int frameIndex = renderer.getFrameIndex();
	        	FrameInfo frameInfo{
	        		frameIndex,
	        		frameTime,
	        		commandBuffer,
	        		camera,
	        		globalDescriptorSets[frameIndex]
	        	};

	        	// update
	        	GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				ubo.view = camera.getView();
	        	uboBuffers[frameIndex]->writeToBuffer(&ubo);
	        	uboBuffers[frameIndex]->flush();

	        	// render
	            renderer.beginSwapChainRenderPass(commandBuffer);
	            renderSystem.renderGameObjects(frameInfo, gameObjects);
				pointLightSystem.render(frameInfo);
	            renderer.endSwapChainRenderPass(commandBuffer);
	            renderer.endFrame();
	        }
	    }
	    vkDeviceWaitIdle(engineDevice.device());
	}

private:
	void loadGameObjects(){
		std::shared_ptr<EngineMesh> model = EngineMesh::createMeshFromFile(engineDevice, "../models/car.obj");
        auto obj = EngineGameObject::createGameObject();
        obj.mesh = model;
        obj.transform.translation = {0.0f, 0.0f, 0.2f};
        obj.transform.scale = {0.5f, 0.5f, 0.5f};
        gameObjects.push_back(std::move(obj));
    }

	EngineWindow window{width, height, "World"};
    EngineDevice engineDevice{window};
    Renderer renderer{window, engineDevice};

    std::unique_ptr<EngineDescriptorPool> globalPool{};
    std::vector<EngineGameObject> gameObjects;
};
} // namespace
#endif