#include "application.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

namespace engine {
	struct SimplePushConstantData {
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	application::application() {
		loadEntities();
		createPipelineLayout();
		recreateSwapchain();
		createCommandBuffers();
	}

	application::~application() {
		vkDestroyPipelineLayout(deviceInstance.getDevice(), pipelineLayout, nullptr);
	}

	void application::run() {
		while (!windowInstance.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(deviceInstance.getDevice());
	}

	void application::loadEntities() {
		std::vector<model::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto modelInstance = std::make_shared<model>(deviceInstance, vertices);
		auto triangle = entity::createEntity();
		triangle.modelInstance = modelInstance;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>(); // in radians
		entities.push_back(std::move(triangle));
	}

	void application::createPipelineLayout() {
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

	void application::createPipeline() {
		// create a config for the pipeline
		PipelineConfigInfo pipelineConfig = {};
		pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = swapchainInstance->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineInstance = std::make_unique<pipeline>(deviceInstance, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}

	void application::recreateSwapchain() {
		// get the current window size
		auto extent = windowInstance.getExtent();

		// update extent while size is valid, will pause and wait during minimization
		while (extent.width == 0 || extent.height == 0) {
			extent = windowInstance.getExtent();
			glfwWaitEvents();
		}

		// wait until the current swap chain is no longer being used before we create another
		vkDeviceWaitIdle(deviceInstance.getDevice());
		// swapchainInstance = nullptr; // temporary fix; two swap chains can't coexist on the same window, so destroy the old one first

		if (swapchainInstance == nullptr) {
			swapchainInstance = std::make_unique<swapchain>(deviceInstance, extent);
		}
		else {
			swapchainInstance = std::make_unique<swapchain>(deviceInstance, extent, std::move(swapchainInstance));
			if (swapchainInstance->getImageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();
	}

	void application::createCommandBuffers() {
		// // resize the container holding the command buffers
		commandBuffers.resize(swapchainInstance->getImageCount());

		// allocate the command buffers by specifying the command pool and number of buffers to allocate
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = deviceInstance.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		// create the command buffers
		if (vkAllocateCommandBuffers(deviceInstance.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void application::freeCommandBuffers() {
		vkFreeCommandBuffers(deviceInstance.getDevice(), deviceInstance.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void application::recordCommandBuffer(int imageIndex) {
		// begin recording command buffers
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// start defining a render pass, creating a framebuffer for each swap chain image where it is specified as a color attachment
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapchainInstance->getRenderPass();
		renderPassInfo.framebuffer = swapchainInstance->getFrameBuffer(imageIndex);

		// define the size of the render area
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchainInstance->getSwapchainExtent();

		// define clear values to use for the color attachment
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.01f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// record to our command buffer to begin the render pass
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// configure the dynamic viewport and scissor
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchainInstance->getSwapchainExtent().width);
		viewport.height = static_cast<float>(swapchainInstance->getSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapchainInstance->getSwapchainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		// render the entities
		renderEntities(commandBuffers[imageIndex]);

		// end the render pass and finish recording the command buffer
		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void application::renderEntities(VkCommandBuffer commandBuffer) {
		pipelineInstance->bind(commandBuffer);

		// loop through all entities and record their binds and draws to the command buffer
		for (auto& entityInstance : entities) {
			entityInstance.transform2d.rotation = glm::mod(entityInstance.transform2d.rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push = {};
			push.offset = entityInstance.transform2d.translation;
			push.color = entityInstance.color;
			push.transform = entityInstance.transform2d.mat2();

			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			entityInstance.modelInstance->bind(commandBuffer);
			entityInstance.modelInstance->draw(commandBuffer);
		}
	}

	void application::drawFrame() {
		// acquire an image from the swap chain
		uint32_t imageIndex;
		auto result = swapchainInstance->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// record and  submit the command buffer
		recordCommandBuffer(imageIndex);
		result = swapchainInstance->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowInstance.wasWindowResized()) {
			windowInstance.resetWindowResizedFlag();
			recreateSwapchain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}
}