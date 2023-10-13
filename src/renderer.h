#ifndef RENDERER_H
#define RENDERER_H

#include "engine_window.h"
#include "engine_swap_chain.h"
#include "engine_device.h"
#include "engine_mesh.h"

#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <array>
#include <cassert>

namespace Engine{
class Renderer{
public:

	Renderer(EngineWindow &_window, EngineDevice &device) : window{_window}, engineDevice{device}
	{
		recreateSwapChain();
		createCommandBuffers();
	}


	~Renderer() {freeCommandBuffers();}
	
	Renderer(const Renderer &) = delete;
	Renderer &operator=(const Renderer &) = delete;	

	VkRenderPass getSwapChainRenderPass() const {return engineSwapChain->getRenderPass();}
	float getAspectRatio() const {return engineSwapChain->extentAspectRatio();}

	bool isFrameInProgress() const {return isFrameStarted;}

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameStarted && "Cannot get command buffer when frame is not in progress!");
		return commandBuffers[currentFrameIndex];
	}

	int getFrameIndex() const {
		assert(isFrameStarted && "Can't get frame index when frame not in progress!");
		return currentFrameIndex;
	}

	VkCommandBuffer beginFrame(){
		assert(!isFrameStarted && "Can't call beginFrame while already in progress!");

		auto result = engineSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR){
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
			throw std::runtime_error("failed to aquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void endFrame(){
		assert(isFrameStarted && "Can't call end frame while frame is not in progress!");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = engineSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()){
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS){
			throw std::runtime_error("failed to present swap chain image!");
		}	
		isFrameStarted = false;	
		currentFrameIndex = (currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = engineSwapChain->getRenderPass();
		renderPassInfo.framebuffer = engineSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = engineSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.69f, 0.84f, 0.89f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);	
		
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(engineSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(engineSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, engineSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer){
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");
		vkCmdEndRenderPass(commandBuffer);
	}


private:


	void createCommandBuffers() {
		commandBuffers.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = engineDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(engineDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void freeCommandBuffers(){
		vkFreeCommandBuffers(
			engineDevice.device(), 
			engineDevice.getCommandPool(), 
			static_cast<uint32_t>(commandBuffers.size()), 
			commandBuffers.data());
		commandBuffers.clear();
	}

	void recreateSwapChain(){
		auto extent = window.getExtent();
		while (extent.width == 0 || extent.height == 0){
			extent = window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(engineDevice.device());

		if (engineSwapChain == nullptr){
			engineSwapChain = std::make_unique<EngineSwapChain>(engineDevice, extent);
		}
		else{
			std::shared_ptr<EngineSwapChain> oldSwapChain = std::move(engineSwapChain);
			engineSwapChain = std::make_unique<EngineSwapChain>(engineDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*engineSwapChain.get())){
				throw std::runtime_error("Swap chain image format has changed!");
			}
		}

		// we'll come back here
	}

	EngineWindow& window;
    EngineDevice& engineDevice;
    std::unique_ptr<EngineSwapChain> engineSwapChain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImageIndex;
    int currentFrameIndex{0};
    bool isFrameStarted = false;
};



} // namespace


#endif