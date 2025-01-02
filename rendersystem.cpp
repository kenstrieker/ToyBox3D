#include "rendersystem.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

namespace engine {
	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	rendersystem::rendersystem(device& deviceInstance, VkRenderPass renderPass) : deviceInstance{ deviceInstance } {
		createPipelineLayout();
		createPipeline(renderPass);
	}

	rendersystem::~rendersystem() {
		vkDestroyPipelineLayout(deviceInstance.getDevice(), pipelineLayout, nullptr);
	}

	void rendersystem::createPipelineLayout() {
		// create a push constant range
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		// fill out the VkPipelineLayoutCreateInfo struct
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		// create the pipeline layout
		if (vkCreatePipelineLayout(deviceInstance.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void rendersystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		// create a config for the pipeline
		PipelineConfigInfo pipelineConfig = {};
		pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineInstance = std::make_unique<pipeline>(deviceInstance, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}

	void rendersystem::renderEntities(FrameInfo& frameInfo, std::vector<entity>& entities) {
		pipelineInstance->bind(frameInfo.commandBuffer);

		auto projectionView = frameInfo.cameraInstance.getProjection() * frameInfo.cameraInstance.getView();

		// loop through all entities and record their binds and draws to the command buffer
		for (auto& entityInstance : entities) {
			SimplePushConstantData push = {};
			auto modelMatrix = entityInstance.transform.mat4();
			push.transform = projectionView * modelMatrix;
			push.normalMatrix = entityInstance.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			entityInstance.modelInstance->bind(frameInfo.commandBuffer);
			entityInstance.modelInstance->draw(frameInfo.commandBuffer);
		}
	}
}