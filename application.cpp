#include "application.hpp"
#include "camera.hpp"
#include "rendersystem.hpp"
#include "pointlightsystem.hpp"
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

namespace ToyBox {
    Application::Application() {
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();
        loadEntities(); 
    }

    Application::~Application() {}

	void Application::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(device).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
        }

		RenderSystem renderSys{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSys{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        Camera camera = {};
        
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        // store the camera's current state
        auto viewerEntity = Entity::createEntity();
        viewerEntity.transform.translation.z = -2.5f;
        Input cameraController = {};

        // for game loop timing
        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerEntity);
            camera.setViewYXZ(viewerEntity.transform.translation, viewerEntity.transform.rotation);
            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
			if (auto commandBuffer = renderer.beginFrame()) {
                // prepare and update entities in memory
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameEntities };
                GlobalUbo ubo = {};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSys.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSys.renderEntities(frameInfo);
                pointLightSys.render(frameInfo);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.getDevice());
	}

    void Application::loadEntities() {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "A:\\Dev\\Libraries\\models\\tree.obj");

        auto tree = Entity::createEntity();
        tree.model = model;
        tree.transform.translation = { .0f, 1.0f, 0.f };
        tree.transform.scale = { .05f, .05f, .05f };
        tree.transform.rotation = { .0f, .0f, 3.14f };
        gameEntities.emplace(tree.getId(), std::move(tree));

        model = Model::createModelFromFile(device, "A:\\Dev\\Libraries\\models\\flat_vase.obj");
        
        auto vase = Entity::createEntity();
        vase.model = model;
        vase.transform.translation = { .0f, 2.08f, 0.f };
        vase.transform.scale = { 3.f, 3.f, 3.f };
        gameEntities.emplace(vase.getId(), std::move(vase));

        model = Model::createModelFromFile(device, "A:\\Dev\\Libraries\\models\\quad.obj");

        auto floor = Entity::createEntity();
        floor.model = model;
        floor.transform.translation = { .0f, 2.08f, 0.f };
        floor.transform.scale = { 5.f, 5.f, 5.f };
        gameEntities.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors {
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = Entity::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameEntities.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}