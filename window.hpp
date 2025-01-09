#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace ToyBox {
	class Window {
	public:
		Window(int width, int height, std::string title); // constructor
		~Window(); // clean up resources, destroy and terminate GLFW

		Window(const Window&) = delete; // to avoid dangling pointers
		Window& operator = (const Window&) = delete; // to avoid dangling pointers

		bool shouldClose() { return glfwWindowShouldClose(window); } // checks if we should close the window
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; } // returns the width and height
		bool wasWindowResized() { return framebufferResized; } // check if the window was resized
		void resetWindowResizedFlag() { framebufferResized = false; } // reset the window resized flag
		GLFWwindow* getGLFWwindow() const { return window; } // return the handle to the window instance
		void createWindowSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface); // lets GLFW handle creating the surface to present rendered images to

	private:
		static void framebufferResizeCallback(GLFWwindow* windowInstance, int width, int height); // to resize the window
		void init();
		int width; // window width
		int height; // window height
		bool framebufferResized = false; // for checking if the user resized the window
		std::string title; // window title
		GLFWwindow* window; // pointer to the window instance
	};
}