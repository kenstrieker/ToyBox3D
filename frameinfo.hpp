#pragma once
#include "camera.hpp"
#include "entity.hpp"
#include <vulkan/vulkan.h>

namespace engine {
#define MAX_LIGHTS 10
	struct PointLight {
		glm::vec4 position = {};
		glm::vec4 color = {};
	};

	// struct to create a global uniform buffer
	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // r, g, b, intensity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	// struct for wrapping all frame-relevant data into a single object
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		camera& cameraInstance;
		VkDescriptorSet globalDescriptorSet;
		entity::Map& gameEntities;
	};
}