#pragma once
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include <memory>
#include <vector>

namespace engine {
	class application {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		application();
		~application();

		application(const application&) = delete;
		application& operator = (const application&) = delete;

		void run(); // main event loop function

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

		window windowInstance{ WIDTH, HEIGHT, "VulkanGame" };
		device deviceInstance{ windowInstance };
		swapchain swapchainInstance{ deviceInstance, windowInstance.getExtent() };
		std::unique_ptr<pipeline> pipelineInstance;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
	};
}