#include "application.hpp"
#include <stdexcept>
#include <array>

namespace engine {
	application::application() {
		loadModels();
		createPipelineLayout();
		createPipeline();
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

	void application::loadModels() {
		std::vector<model::Vertex> vertices{ {{0.0f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}} };

		modelInstance = std::make_unique<model>(deviceInstance, vertices);
	}

	void application::createPipelineLayout() {
		// fill out the VkPipelineLayoutCreateInfo struct
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		// create the pipeline layout
		if (vkCreatePipelineLayout(deviceInstance.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void application::createPipeline() {
		// create a config for the pipeline
		auto pipelineConfig = pipeline::defaultPipelineConfigInfo(swapchainInstance.getWidth(), swapchainInstance.getHeight());
		pipelineConfig.renderPass = swapchainInstance.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineInstance = std::make_unique<pipeline>(deviceInstance, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}

	void application::createCommandBuffers() {
		// // resize the container holding the command buffers
		commandBuffers.resize(swapchainInstance.getImageCount());

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

		// iterate through the command buffers for recording
		for (int i = 0; i < commandBuffers.size(); i++) {
			// begin recording command buffers
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			// start defining a render pass, creating a framebuffer for each swap chain image where it is specified as a color attachment
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapchainInstance.getRenderPass();
			renderPassInfo.framebuffer = swapchainInstance.getFrameBuffer(i);

			// define the size of the render area
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapchainInstance.getSwapchainExtent();

			// define clear values to use for the color attachment
			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			// record to our command buffer to begin the render pass
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// bind the graphics pipeline and model, and issue the draw command
			pipelineInstance->bind(commandBuffers[i]);
			modelInstance->bind(commandBuffers[i]);
			modelInstance->draw(commandBuffers[i]);

			// end the render pass and finish recording the command buffer
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void application::drawFrame() {
		// acquire an image from the swap chain
		uint32_t imageIndex;
		auto result = swapchainInstance.acquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// submit the command buffer
		result = swapchainInstance.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}
}