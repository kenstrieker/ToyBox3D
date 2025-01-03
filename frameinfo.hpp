#pragma once
#include "camera.hpp"
#include "entity.hpp"
#include <vulkan/vulkan.h>

namespace engine {
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