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
		alignas(16) glm::vec3 color;
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

	void rendersystem::renderEntities(VkCommandBuffer commandBuffer, std::vector<entity>& entities, const camera& cameraInstance) {
		pipelineInstance->bind(commandBuffer);

		auto projectionView = cameraInstance.getProjection() * cameraInstance.getView();

		// loop through all entities and record their binds and draws to the command buffer
		for (auto& entityInstance : entities) {
			entityInstance.transform.rotation.y = glm::mod(entityInstance.transform.rotation.y + 0.01f, glm::two_pi<float>());
			entityInstance.transform.rotation.x = glm::mod(entityInstance.transform.rotation.x + 0.005f, glm::two_pi<float>());
			entityInstance.transform.rotation.z = glm::mod(entityInstance.transform.rotation.z + 0.007f, glm::two_pi<float>());

			SimplePushConstantData push = {};
			push.color = entityInstance.color;
			push.transform = projectionView * entityInstance.transform.mat4();

			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			entityInstance.modelInstance->bind(commandBuffer);
			entityInstance.modelInstance->draw(commandBuffer);
		}
	}
}