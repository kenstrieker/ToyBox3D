#include "rendersystem.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

namespace engine {
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	rendersystem::rendersystem(device& deviceInstance, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : deviceInstance{ deviceInstance } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	rendersystem::~rendersystem() {
		vkDestroyPipelineLayout(deviceInstance.getDevice(), pipelineLayout, nullptr);
	}

	void rendersystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		// create a push constant range
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		// fill out the VkPipelineLayoutCreateInfo struct
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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

	void rendersystem::renderEntities(FrameInfo& frameInfo) {
		pipelineInstance->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		// loop through all entities and record their binds and draws to the command buffer
		for (auto& kv : frameInfo.gameEntities) {
			auto& entityInstance = kv.second;
			if (entityInstance.modelInstance == nullptr) continue;
			SimplePushConstantData push = {};
			push.modelMatrix = entityInstance.transform.mat4();
			push.normalMatrix = entityInstance.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			entityInstance.modelInstance->bind(frameInfo.commandBuffer);
			entityInstance.modelInstance->draw(frameInfo.commandBuffer);
		}
	}
}