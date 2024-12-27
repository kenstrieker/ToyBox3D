#pragma once
#include "window.hpp"
#include <string>
#include <vector>
#include <optional>

namespace engine {
	// struct for checking surface capabilities, surface formats, and available presentation modes for the swap chain
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// struct for bundled queue indices to submit commands to
	struct QueueFamilyIndices {
		uint32_t graphicsFamily; // could use std::optional for this, but will need some refactoring with current implementation
		uint32_t presentFamily; // same as above with std::optional
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
	};

	class device {

	public:
#ifdef NDEBUG // not to be compiled in debug mode
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		device(window& windowInstance); // constructor
		~device(); // destructor

		// not copyable or movable
		device(const device&) = delete;
		device& operator = (const device&) = delete;
		device(device&&) = delete;
		device& operator = (device&&) = delete;

		// getters for class members
		VkCommandPool getCommandPool() { return commandPool; }
		VkDevice getDevice() { return device_; }
		VkSurfaceKHR getSurface() { return surface_; }
		VkQueue getGraphicsQueue() { return graphicsQueue_; }
		VkQueue getPresentQueue() { return presentQueue_; }

		SwapChainSupportDetails getSwapchainSupport() { return querySwapchainSupport(physicalDevice); } // get swap chain support details for the physical device
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties); // find the right type of memory to use based on the vertex buffer and our own app requirements
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); } // look for all the queue families we need
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); // initialize and return a buffer
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
		void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkPhysicalDeviceProperties deviceProperties;

	private:
		void createInstance(); // initialize the Vulkan library
		void setupDebugMessenger(); // to handle the debug messenger and set up validation layers
		void createSurface(); // relies on GLFW, connection between the window and Vulkan's ability to display
		void pickPhysicalDevice(); // to select a graphics card in the system that supports necessary features
		void createLogicalDevice(); // to describe what features to use so that the graphics card can be interfaced with
		void createCommandPool(); // for managing the memory that is used to store command buffers

		bool isDeviceSuitable(VkPhysicalDevice deviceInstance); // evaluate the suitability of a device by querying for some details
		std::vector<const char*> getRequiredExtensions(); // get necessary extensions from GLFW to interface with window based on whether validation layers are enabled
		bool checkValidationLayerSupport(); // check if all of the validation requested layers are available
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice deviceInstance); // check which queue families are supported by the device, and which device supports the necessary commands
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo); // fills in a structure with debug messenger and callback details
		void hasGlfwRequiredInstanceExtensions(); // check if required GLFW extensions are present
		bool checkDeviceExtensionSupport(VkPhysicalDevice deviceInstance); // called from isDeviceSuitable as an additonal check
		SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice deviceInstance); // to populate the SwapChainSupportDetails struct

		VkInstance vulkanInstance; // data member to handle Vulkan instance
		VkDebugUtilsMessengerEXT debugMessenger; // a handle to tell Vulkan about the callback function, needs to be created and destroyed
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // a handle to store the graphics card that will be implicitly destroyed when VkInstance is destroyed
		window& windowInstance; // a handle to store the window instance
		VkCommandPool commandPool; // a handle to store the command pool to manage buffer/command buffer memory
		
		VkDevice device_;
		VkSurfaceKHR surface_; // a handle to store the surface to present rendered images to
		VkQueue graphicsQueue_; // a handle to store the graphics queue
		VkQueue presentQueue_; // a handle to store the presentation queue

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }; // standard validation is bundled into this layer included in the SDK
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // list of required device extensions
	};
}