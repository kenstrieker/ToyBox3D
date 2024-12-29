#pragma once
#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include <cassert>
#include <memory>
#include <vector>

namespace engine {
	class renderer {
	public:
		renderer(window& windowInstance, device& deviceInstance); // constructor
		~renderer(); // destructor

		// not copyable or movable
		renderer(const renderer&) = delete;
		renderer& operator = (const renderer&) = delete;

		VkRenderPass getSwapchainRenderPass() const { return swapchainInstance->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame(); // start a frame
		VkCommandBuffer endFrame(); // end a frame
		void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers(); // allocate command buffers from the command pool
		void freeCommandBuffers(); // deallocate command buffers
		void recreateSwapchain(); // recreate the swap chain (for example, when resizing the window)

		window& windowInstance;; // a handle for the window instance
		device& deviceInstance; // a handle for the device instance
		std::unique_ptr<swapchain> swapchainInstance; // a handle for the swap chain instance
		std::vector<VkCommandBuffer> commandBuffers; // a handle for the command buffers
		uint32_t currentImageIndex; // a handle for the index of the current image
		int currentFrameIndex; // keep track of the frame index not tied to the image index
		bool isFrameStarted; // check if the frame has began
	};
}