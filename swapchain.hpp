#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace engine {
	class swapchain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
		swapchain(device& deviceRef, VkExtent2D windowExtent);
		~swapchain();
		
		swapchain(const swapchain&) = delete;
		void operator = (const swapchain&) = delete;

		VkFramebuffer getFrameBuffer(int index) { return swapchainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapchainImageViews[index]; }
		size_t imageCount() { return swapchainImages.size(); }
		VkFormat getSwapchainImageFormat() { return swapchainImageFormat; }
		VkExtent2D getSwapchainExtent() { return swapchainExtent; }
		uint32_t width() { return swapchainExtent.width; }
		uint32_t height() { return swapchainExtent.height; }

		float extentAspectRatio() { return static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height); }

		VkFormat findDepthFormat();
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	private:
		void createSwapchain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;

		std::vector<VkFramebuffer> swapchainFramebuffers;
		VkRenderPass renderPass;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		device& deviceInstance;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapchainInstance;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};
}