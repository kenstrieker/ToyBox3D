#include "application.hpp"
#include "rendersystem.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

namespace engine {
	application::application() {
		loadEntities();
	}

	application::~application() {}

	void application::run() {
		rendersystem rendersys{ deviceInstance, rendererInstance.getSwapchainRenderPass() };

		while (!windowInstance.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = rendererInstance.beginFrame()) {
				rendererInstance.beginSwapchainRenderPass(commandBuffer);
				rendersys.renderEntities(commandBuffer, entities);
				rendererInstance.endSwapchainRenderPass(commandBuffer);
				rendererInstance.endFrame();
			}
		}

		vkDeviceWaitIdle(deviceInstance.getDevice());
	}

	void application::loadEntities() {
		std::vector<model::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto modelInstance = std::make_shared<model>(deviceInstance, vertices);
		auto triangle = entity::createEntity();
		triangle.modelInstance = modelInstance;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>(); // in radians
		entities.push_back(std::move(triangle));
	}
}