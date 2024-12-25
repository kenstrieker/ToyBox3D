#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace engine {
	class window {

	public:
		window(int width, int height, std::string title); // constructor
		~window(); // clean up resources, destroy and terminate GLFW
		window(const window&) = delete; // to avoid dangling pointers
		window& operator = (const window&) = delete; // to avoid dangling pointers
		bool shouldClose() { return glfwWindowShouldClose(windowInstance); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		void createWindowSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface); // lets GLFW handle creating the surface to present rendered images to

	private:
		void init();
		const int width;
		const int height;
		std::string title;
		GLFWwindow* windowInstance;
	};
}