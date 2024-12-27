#include "window.hpp"
#include <stdexcept>

namespace engine {

	window::window(int w, int h, std::string t) : width{ w }, height{ h }, title{ t } {
		init();
	}

	window::~window() {
		glfwDestroyWindow(windowInstance);
		glfwTerminate();
	}

	void window::init() {
		glfwInit(); // initialize the GLFW library
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // specify GLFW to not open with OpenGL context (default)
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // handling resized windows takes special care
		windowInstance = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(windowInstance, this);
		glfwSetFramebufferSizeCallback(windowInstance, framebufferResizeCallback);
	}

	void window::createWindowSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(vulkanInstance, windowInstance, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void window::framebufferResizeCallback(GLFWwindow* windowInstance, int width, int height) {
		auto localWindowInstance = reinterpret_cast<window*>(glfwGetWindowUserPointer(windowInstance));
		localWindowInstance->framebufferResized = true;
		localWindowInstance->width = width;
		localWindowInstance->height = height;
	}
}