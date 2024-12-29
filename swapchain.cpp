#include "swapchain.hpp"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace engine {
	swapchain::swapchain(device& deviceRef, VkExtent2D extent) : deviceInstance{ deviceRef }, windowExtent{ extent } {
		init();
	}

	swapchain::swapchain(device& deviceRef, VkExtent2D extent, std::shared_ptr<swapchain> previous) : deviceInstance{ deviceRef }, windowExtent{ extent }, oldSwapchainInstance{ previous } {
		init();

		// clean up old swap chain since it's no longer needed
		oldSwapchainInstance = nullptr;
	}

	void swapchain::init() {
		createSwapchain();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
	}

	swapchain::~swapchain() {
		for (auto imageView : swapchainImageViews) {
			vkDestroyImageView(deviceInstance.getDevice(), imageView, nullptr);
		}
		swapchainImageViews.clear();

		if (swapchainInstance != nullptr) {
			vkDestroySwapchainKHR(deviceInstance.getDevice(), swapchainInstance, nullptr);
			swapchainInstance = nullptr;
		}

		for (int i = 0; i < depthImages.size(); i++) {
			vkDestroyImageView(deviceInstance.getDevice(), depthImageViews[i], nullptr);
			vkDestroyImage(deviceInstance.getDevice(), depthImages[i], nullptr);
			vkFreeMemory(deviceInstance.getDevice(), depthImageMemorys[i], nullptr);
		}

		for (auto framebuffer : swapchainFramebuffers) {
			vkDestroyFramebuffer(deviceInstance.getDevice(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(deviceInstance.getDevice(), renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(deviceInstance.getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(deviceInstance.getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(deviceInstance.getDevice(), inFlightFences[i], nullptr);
		}
	}

	VkResult swapchain::acquireNextImage(uint32_t* imageIndex) {
		vkWaitForFences(deviceInstance.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		VkResult result = vkAcquireNextImageKHR(deviceInstance.getDevice(), swapchainInstance, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);

		return result;
	}

	VkResult swapchain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(deviceInstance.getDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

		// fill in the VkSubmitInfo struct
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		vkResetFences(deviceInstance.getDevice(), 1, &inFlightFences[currentFrame]);

		// submit the command buffer
		if (vkQueueSubmit(deviceInstance.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// request to present an image to the swap chain
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapchains[] = { swapchainInstance };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = imageIndex;
		auto result = vkQueuePresentKHR(deviceInstance.getPresentQueue(), &presentInfo);
		currentFrame = (currentFrame + 1) & MAX_FRAMES_IN_FLIGHT; // advance to the next frame

		return result;
	}

	void swapchain::createSwapchain() {
		SwapChainSupportDetails swapchainSupport = deviceInstance.getSwapchainSupport();

		// set up swap chain properties
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

		// request 1 + the minimum number of images required to function
		// to not wait on the driverto complete operations before acquiring another image to render to
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

		// make sure not to exceed the maximum number of images, where 0 means that there is no maximum
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		// fill in the swap chain object and specify the surface
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = deviceInstance.getSurface();

		// specify the details of the images
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// specify how to handle images used across multiple queue families
		// important for if the graphics queue family is different from the presentation queue
		QueueFamilyIndices indices = deviceInstance.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };
		if (indices.graphicsFamily != indices.presentFamily) { // images can be used across multiple queue families without explicit ownership transfers
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else { // images are owned by one queue family at a time and ownership must be explicitly transferred before using them in another queue family (best performance)
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // optional
			createInfo.pQueueFamilyIndices = nullptr; // optional
		}

		// specify a certain transform to apply to images; use the current transform to specify no transformation
		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = oldSwapchainInstance == nullptr ? VK_NULL_HANDLE : oldSwapchainInstance->swapchainInstance;

		// create the swap chain
		if (vkCreateSwapchainKHR(deviceInstance.getDevice(), &createInfo, nullptr, &swapchainInstance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// query the final number of images with vkGetSwapchainImagesKHR, then resize the container and call it again to retrieve the handles
		vkGetSwapchainImagesKHR(deviceInstance.getDevice(), swapchainInstance, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(deviceInstance.getDevice(), swapchainInstance, &imageCount, swapchainImages.data());

		// store the format and extent
		swapchainImageFormat = surfaceFormat.format;
		swapchainExtent = extent;
	}

	void swapchain::createImageViews() {
		// resize the list to fit all of the image views to be created
		swapchainImageViews.resize(swapchainImages.size());

		// iterate over the swap chain images to create the image views
		for (size_t i = 0; i < swapchainImages.size(); i++) {
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapchainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = swapchainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			// create the image view
			if (vkCreateImageView(deviceInstance.getDevice(), &viewInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void swapchain::createRenderPass() {
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// set up the color buffer attachment represented by one of the images from the swap chain
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = getSwapchainImageFormat(); // should match the format of the swap chain images
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // stick to 1 sample unless using multisampling
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // before rendering, clear the values to a constant at the start
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // after rendering, store rendered contents in memory to be displayed to the screen
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not using stencil buffer data
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // not using stencil buffer data
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout of image before render pass begins
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout to automatically transition to when render pass finishes

		// set up color buffer attachment subpasses (subsequent rendering operations)
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0; // which attachment to reference to; our array consists of a single color attachment at index 0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // which layout our attachment should have during a subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // explicitly a graphics subpass, not a compute subpass
		subpass.colorAttachmentCount = 1; // index of the attachment is directly refrenced from fragment shader
		subpass.pColorAttachments = &colorAttachmentRef; // specify the reference to the color attachment
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// set up subpass dependencies
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// set up render pass
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(deviceInstance.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void swapchain::createFramebuffers() {
		// resize the container holding the framebuffers
		swapchainFramebuffers.resize(getImageCount());

		// iterate through the image views
		for (size_t i = 0; i < getImageCount(); i++) {
			std::array<VkImageView, 2> attachments = { swapchainImageViews[i], depthImageViews[i] };
			VkExtent2D swapchainExtent = getSwapchainExtent();
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapchainExtent.width;
			framebufferInfo.height = swapchainExtent.height;
			framebufferInfo.layers = 1;

			// create the framebuffers
			if (vkCreateFramebuffer(deviceInstance.getDevice(), &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void swapchain::createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		swapchainDepthFormat = depthFormat;
		VkExtent2D swapchainExtent = getSwapchainExtent();

		depthImages.resize(getImageCount());
		depthImageMemorys.resize(getImageCount());
		depthImageViews.resize(getImageCount());

		for (int i = 0; i < depthImages.size(); i++) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = swapchainExtent.width;
			imageInfo.extent.height = swapchainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			deviceInstance.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = depthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(deviceInstance.getDevice(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void swapchain::createSyncObjects() {
		// resize the containers holding the semaphores
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(getImageCount(), VK_NULL_HANDLE);

		// set up semaphore and fence structs
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// iterate over the frames
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// create the semaphores and fences
			if (vkCreateSemaphore(deviceInstance.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=	VK_SUCCESS ||
				vkCreateSemaphore(deviceInstance.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=	VK_SUCCESS ||
				vkCreateFence(deviceInstance.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkSurfaceFormatKHR swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		// loop through the list and see if the preferred combination of format and colorSpace is available
		for (const auto& availableFormat : availableFormats) {
			// note: load LINEAR images w/ VK_FORMAT_R8G8B8A8_UNORM, load SRGB images w/ VK_FORMAT_R8G8B8A8_SRGB for gamma correction
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		// could rank available formats based on how "good" they are, but in most cases settle with the first specified format
		return availableFormats[0];
	}

	VkPresentModeKHR swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			// choose from VK_PRESENT_MODE_IMMEDIATE_KHR (tearing), VK_PRESENT_MODE_FIFO_KHR (v-sync),
			// VK_PRESENT_MODE_FIFO_RELAXED_KHR (tearing), or VK_PRESENT_MODE_MAILBOX_KHR (triple-buffering)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Present mode: Mailbox" << std::endl;
				return availablePresentMode;
			}
		}

		std::cout << "present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent; // match the resolution of the window
		}
		else {
			VkExtent2D actualExtent = windowExtent;
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkFormat swapchain::findDepthFormat() {
		return deviceInstance.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}