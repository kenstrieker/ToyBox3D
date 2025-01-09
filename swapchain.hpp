#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

namespace ToyBox {
	class SwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // avoids the CPU getting too far ahead of the GPU
		SwapChain(Device& deviceRef, VkExtent2D windowExtent); // constructor
		SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous); // constructor with pointer to previous swap chain
		~SwapChain(); // destructor
		
		// not copyable or movable
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator = (const SwapChain&) = delete;

		// getters for class members
		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		size_t getImageCount() { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		uint32_t getWidth() { return swapChainExtent.width; }
		uint32_t getHeight() { return swapChainExtent.height; }

		float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }

		VkFormat findDepthFormat();
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex); // submit the command buffers and synchronize

		bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
		}

	private:
		void init();
		void createSwapchain(); // create the swap chain
		void createImageViews(); // create the image views
		void createDepthResources();
		void createRenderPass(); // tells the graphics pipeline what layout to expect for the framebuffers
		void createFramebuffers(); // create the framebuffers passed during render pass to reference the image view objects representing the attachments
		void createSyncObjects(); // create the semaphores

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); // find the format settings for the swap chain
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); // find the presentation mode settings for the swap chain
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); // find the extent (~resolution) settings for the swap chain

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkFramebuffer> swapChainFramebuffers; // a handle to hold the framebuffers
		VkRenderPass renderPass; // a handle for the render pass

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages; // a handle for the images
		std::vector<VkImageView> swapChainImageViews; // a handle for image views, describing how to access the image

		Device& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<SwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores; // signals that an image has been acquired from the swapchain and is ready for rendering
		std::vector<VkSemaphore> renderFinishedSemaphores; // signals that rendering has finished and presentation can happen
		std::vector<VkFence> inFlightFences; // fences to ensure only one frame is rendering at a time
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};
}