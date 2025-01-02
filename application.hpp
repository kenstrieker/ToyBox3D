#pragma once
#include "window.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "descriptors.hpp"
#include <memory>
#include <vector>

namespace engine {
	class application {
	public:
		static constexpr int WIDTH = 800; // window width
		static constexpr int HEIGHT = 600; // window height

		application(); // constructor
		~application(); // destructor

		// not copyable or movable
		application(const application&) = delete;
		application& operator = (const application&) = delete;

		void run(); // main event loop function

	private:
		void loadEntities(); // load the entities

		window windowInstance{ WIDTH, HEIGHT, "VulkanGame" }; // a handle for the window instance
		device deviceInstance{ windowInstance }; // a handle for the device instance
		std::vector<entity> entities; // a handle for the entity objects
		std::unique_ptr<descriptorPool> globalPool = {}; // a handle for the descriptor pool
		renderer rendererInstance{ windowInstance, deviceInstance }; // a handle for the renderer
	};
}