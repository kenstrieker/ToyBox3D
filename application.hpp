#pragma once
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "model.hpp"
#include <memory>
#include <vector>

namespace engine {
	class application {
	public:
		static constexpr int WIDTH = 800; // window width
		static constexpr int HEIGHT = 600; // window height

		application(); // constructor
		~application(); // destructor

		// not copyable or movable
		application(const application&) = delete;
		application& operator = (const application&) = delete;

		void run(); // main event loop function

	private:
		void loadModels();
		void createPipelineLayout(); // create a pipeline layout
		void createPipeline(); // create a pipeline
		void createCommandBuffers(); // allocate command buffers from the command pool
		void drawFrame(); // draw a frame

		window windowInstance{ WIDTH, HEIGHT, "VulkanGame" }; // a handle for the window instance
		device deviceInstance{ windowInstance }; // a handle for the device instance
		swapchain swapchainInstance{ deviceInstance, windowInstance.getExtent() }; // a handle for the swap chain instance
		std::unique_ptr<pipeline> pipelineInstance; // a handle for the pipeline instance
		VkPipelineLayout pipelineLayout; // a handle for the pipeline layout
		std::vector<VkCommandBuffer> commandBuffers; // a handle for the command buffers
		std::unique_ptr<model> modelInstance;
	};
}