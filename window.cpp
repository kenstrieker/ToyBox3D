#include "window.hpp"
#include <stdexcept>

namespace ToyBox {
	Window::Window(int w, int h, std::string t) : width{ w }, height{ h }, title{ t } {
		init();
	}

	Window::~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::init() {
		glfwInit(); // initialize the GLFW library
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // specify GLFW to not open with OpenGL context (default)
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // handling resized windows takes special care
		window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::createWindowSurface(VkInstance vulkan, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(vulkan, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto localWindowInstance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		localWindowInstance->framebufferResized = true;
		localWindowInstance->width = width;
		localWindowInstance->height = height;
	}
}