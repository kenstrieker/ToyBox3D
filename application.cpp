#include "application.hpp"
#include "camera.hpp"
#include "rendersystem.hpp"
#include "buffer.hpp"
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
    // struct to create a global uniform buffer
    struct GlobalUbo {
        alignas(16) glm::mat4 projectionView{ 1.f };
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
    };

	application::application() {
        globalPool = descriptorPool::Builder(deviceInstance).setMaxSets(swapchain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain::MAX_FRAMES_IN_FLIGHT).build();
        loadEntities(); 
    }

	application::~application() {}

	void application::run() {
        std::vector<std::unique_ptr<buffer>> uboBuffers(swapchain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<buffer>(deviceInstance, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = descriptorSetLayout::Builder(deviceInstance).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build();
        std::vector<VkDescriptorSet> globalDescriptorSets(swapchain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            descriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
        }

		rendersystem rendersys{ deviceInstance, rendererInstance.getSwapchainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
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
                // prepare and update entities in memory
                int frameIndex = rendererInstance.getFrameIndex();
                GlobalUbo ubo = {};
                ubo.projectionView = cameraInstance.getProjection() * cameraInstance.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, cameraInstance, globalDescriptorSets[frameIndex] };
				rendererInstance.beginSwapchainRenderPass(commandBuffer);
				rendersys.renderEntities(frameInfo, entities);
				rendererInstance.endSwapchainRenderPass(commandBuffer);
				rendererInstance.endFrame();
			}
		}

		vkDeviceWaitIdle(deviceInstance.getDevice());
	}

    void application::loadEntities() {
        std::shared_ptr<model> modelInstance = model::createModelFromFile(deviceInstance, "A:\\Dev\\Libraries\\models\\tree.obj");
        auto cube = entity::createEntity();
        cube.modelInstance = modelInstance;
        // cube.transform.translation = { .0f, .0f, 2.5f };
        // cube.transform.scale = { .5f, .5f, .5f };
        cube.transform.translation = { .0f, 1.0f, 2.5f };
        cube.transform.scale = { .05f, .05f, .05f };
        cube.transform.rotation = { .0f, .0f, 3.14f };
        entities.push_back(std::move(cube));
    }
}