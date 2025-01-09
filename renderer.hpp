#pragma once
#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include <cassert>
#include <memory>
#include <vector>

namespace ToyBox {
	class Renderer {
	public:
		Renderer(Window& window, Device& device); // constructor
		~Renderer(); // destructor

		// not copyable or movable
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
		float getAspectRatio() const { return swapChain->extentAspectRatio(); }
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
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers(); // allocate command buffers from the command pool
		void freeCommandBuffers(); // deallocate command buffers
		void recreateSwapChain(); // recreate the swap chain (for example, when resizing the window)

		Window& window; // a handle for the window instance
		Device& device; // a handle for the device instance
		std::unique_ptr<SwapChain> swapChain; // a handle for the swap chain instance
		std::vector<VkCommandBuffer> commandBuffers; // a handle for the command buffers
		uint32_t currentImageIndex; // a handle for the index of the current image
		int currentFrameIndex; // keep track of the frame index not tied to the image index
		bool isFrameStarted; // check if the frame has began
	};
}