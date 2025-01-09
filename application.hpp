#pragma once
#include "window.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "descriptors.hpp"
#include <memory>
#include <vector>

namespace ToyBox {
	class Application {
	public:
		static constexpr int WIDTH = 1920; // window width
		static constexpr int HEIGHT = 1080; // window height

		Application(); // constructor
		~Application(); // destructor

		// not copyable or movable
		Application(const Application&) = delete;
		Application& operator = (const Application&) = delete;

		void run(); // main event loop function

	private:
		void loadEntities(); // load the entities

		Window window{ WIDTH, HEIGHT, "VulkanGame" }; // a handle for the window instance
		Device device{ window }; // a handle for the device instance
		Entity::Map gameEntities; // a handle for the entity objects
		std::unique_ptr<DescriptorPool> globalPool = {}; // a handle for the descriptor pool
		Renderer renderer{ window, device }; // a handle for the renderer
	};
}