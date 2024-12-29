#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

namespace engine {
	class swapchain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // avoids the CPU getting too far ahead of the GPU
		swapchain(device& deviceRef, VkExtent2D windowExtent); // constructor
		swapchain(device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<swapchain> previous); // constructor with pointer to previous swap chain
		~swapchain(); // destructor
		
		// not copyable or movable
		swapchain(const swapchain&) = delete;
		swapchain& operator = (const swapchain&) = delete;

		// getters for class members
		VkFramebuffer getFrameBuffer(int index) { return swapchainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapchainImageViews[index]; }
		size_t getImageCount() { return swapchainImages.size(); }
		VkFormat getSwapchainImageFormat() { return swapchainImageFormat; }
		VkExtent2D getSwapchainExtent() { return swapchainExtent; }
		uint32_t getWidth() { return swapchainExtent.width; }
		uint32_t getHeight() { return swapchainExtent.height; }

		float extentAspectRatio() { return static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height); }

		VkFormat findDepthFormat();
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex); // submit the command buffers and synchronize

		bool compareSwapFormats(const swapchain& swapchainInstance) const {
			return swapchainInstance.swapchainDepthFormat == swapchainDepthFormat && swapchainInstance.swapchainImageFormat == swapchainImageFormat;
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

		VkFormat swapchainImageFormat;
		VkFormat swapchainDepthFormat;
		VkExtent2D swapchainExtent;

		std::vector<VkFramebuffer> swapchainFramebuffers; // a handle to hold the framebuffers
		VkRenderPass renderPass; // a handle for the render pass

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapchainImages; // a handle for the images
		std::vector<VkImageView> swapchainImageViews; // a handle for image views, describing how to access the image

		device& deviceInstance;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapchainInstance;
		std::shared_ptr<swapchain> oldSwapchainInstance;

		std::vector<VkSemaphore> imageAvailableSemaphores; // signals that an image has been acquired from the swapchain and is ready for rendering
		std::vector<VkSemaphore> renderFinishedSemaphores; // signals that rendering has finished and presentation can happen
		std::vector<VkFence> inFlightFences; // fences to ensure only one frame is rendering at a time
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};
}