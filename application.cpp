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
        glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // r, g, b, intensity
        glm::vec3 lightPosition{ -1.f };
        alignas(16) glm::vec4 lightColor{ 1.f }; // r, g, b, intensity
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

        auto globalSetLayout = descriptorSetLayout::Builder(deviceInstance).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
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
        viewerEntity.transform.translation.z = -2.5f;
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
            cameraInstance.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
			if (auto commandBuffer = rendererInstance.beginFrame()) {
                // prepare and update entities in memory
                int frameIndex = rendererInstance.getFrameIndex();
                GlobalUbo ubo = {};
                ubo.projectionView = cameraInstance.getProjection() * cameraInstance.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, cameraInstance, globalDescriptorSets[frameIndex], gameEntities };
				rendererInstance.beginSwapchainRenderPass(commandBuffer);
				rendersys.renderEntities(frameInfo);
				rendererInstance.endSwapchainRenderPass(commandBuffer);
				rendererInstance.endFrame();
			}
		}

		vkDeviceWaitIdle(deviceInstance.getDevice());
	}

    void application::loadEntities() {
        std::shared_ptr<model> modelInstance = model::createModelFromFile(deviceInstance, "A:\\Dev\\Libraries\\models\\tree.obj");

        auto tree = entity::createEntity();
        tree.modelInstance = modelInstance;
        tree.transform.translation = { .0f, 1.0f, 0.f };
        tree.transform.scale = { .05f, .05f, .05f };
        tree.transform.rotation = { .0f, .0f, 3.14f };
        gameEntities.emplace(tree.getId(), std::move(tree));

        modelInstance = model::createModelFromFile(deviceInstance, "A:\\Dev\\Libraries\\models\\flat_vase.obj");
        
        auto vase = entity::createEntity();
        vase.modelInstance = modelInstance;
        vase.transform.translation = { .0f, 2.08f, 0.f };
        vase.transform.scale = { 3.f, 3.f, 3.f };
        gameEntities.emplace(vase.getId(), std::move(vase));

        modelInstance = model::createModelFromFile(deviceInstance, "A:\\Dev\\Libraries\\models\\quad.obj");

        auto floor = entity::createEntity();
        floor.modelInstance = modelInstance;
        floor.transform.translation = { .0f, 2.08f, 0.f };
        floor.transform.scale = { 5.f, 5.f, 5.f };
        gameEntities.emplace(floor.getId(), std::move(floor));
    }
}