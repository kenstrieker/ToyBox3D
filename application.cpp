#include "application.hpp"
#include "camera.hpp"
#include "rendersystem.hpp"
#include "input.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
#include <chrono>
#include <cassert>

namespace engine {
	application::application() {
		loadEntities();
	}

	application::~application() {}

	void application::run() {
		rendersystem rendersys{ deviceInstance, rendererInstance.getSwapchainRenderPass() };
        camera cameraInstance = {};
        
        cameraInstance.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        // store the camera's current state
        auto viewerEntity = entity::createEntity();
        input cameraController = {};

        // for game loop timing
        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!windowInstance.shouldClose()) {
			glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = glm::min(frameTime, 1.f);
            cameraController.moveInPlaneXZ(windowInstance.getGLFWwindow(), frameTime, viewerEntity);
            cameraInstance.setViewYXZ(viewerEntity.transform.translation, viewerEntity.transform.rotation);
            float aspect = rendererInstance.getAspectRatio();
            cameraInstance.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
			if (auto commandBuffer = rendererInstance.beginFrame()) {
				rendererInstance.beginSwapchainRenderPass(commandBuffer);
				rendersys.renderEntities(commandBuffer, entities, cameraInstance);
				rendererInstance.endSwapchainRenderPass(commandBuffer);
				rendererInstance.endFrame();
			}
		}

		vkDeviceWaitIdle(deviceInstance.getDevice());
	}

    // temporary helper function to create a 1x1x1 cube centered at the offset
    std::unique_ptr<model> createCubeModel(device& deviceInstance, glm::vec3 offset) {
        std::vector<model::Vertex> vertices{
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };

        for (auto& v : vertices) {
            v.position += offset;
        }

        return std::make_unique<model>(deviceInstance, vertices);
    }

    void application::loadEntities() {
        std::shared_ptr<model> modelInstance = createCubeModel(deviceInstance, { .0f, .0f, .0f });
        auto cube = entity::createEntity();
        cube.modelInstance = modelInstance;
        cube.transform.translation = { .0f, .0f, 2.5f };
        cube.transform.scale = { .5f, .5f, .5f };
        entities.push_back(std::move(cube));
    }
}