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
		bool shouldClose() { return glfwWindowShouldClose(windowInstance); } // checks if we should close the window
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; } // returns the width and height
		bool wasWindowResized() { return framebufferResized; } // check if the window was resized
		void resetWindowResizedFlag() { framebufferResized = false; } // reset the window resized flag
		void createWindowSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface); // lets GLFW handle creating the surface to present rendered images to

	private:
		static void framebufferResizeCallback(GLFWwindow* windowInstance, int width, int height); // to resize the window
		void init();
		int width; // window width
		int height; // window height
		bool framebufferResized = false; // for checking if the user resized the window
		std::string title; // window title
		GLFWwindow* windowInstance; // pointer to the window instance
	};
}