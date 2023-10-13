#ifndef ENGINE_POINT_LIGHT_SYSTEM_H
#define ENGINE_POINT_LIGHT_SYSTEM_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "engine_pipeline.h"
#include "engine_device.h"
#include "engine_game_object.h"
#include "engine_camera.h"
#include "engine_frame_info.h"


#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <array>

namespace Engine{


class PointLightSystem{
public:

	PointLightSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : engineDevice{device} 
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}


	~PointLightSystem() {vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);};
	
	PointLightSystem(const PointLightSystem &) = delete;
	PointLightSystem &operator=(const PointLightSystem &) = delete;	


	void render(FrameInfo &frameInfo)
	{
		enginePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 
			1, 
			&frameInfo.globalDescriptorSet,
			0, 
			nullptr);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}


private:

	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

		// VkPushConstantRange pushConstantRange{};
		// pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// pushConstantRange.offset = 0;
		// pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};



		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;	
		pipelineConfig.pipelineLayout = pipelineLayout;
		enginePipeline = std::make_unique<EnginePipeline>(
			engineDevice, 
			"../shaders/point_light.vert.spv", 
			"../shaders/point_light.frag.spv", 
			pipelineConfig);
	}


    EngineDevice& engineDevice;
    std::unique_ptr<EnginePipeline> enginePipeline;
    VkPipelineLayout pipelineLayout;
};



} // namespace


#endif