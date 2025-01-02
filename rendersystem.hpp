#pragma once
#include "camera.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "entity.hpp"
#include <memory>
#include <vector>

namespace engine {
	// struct for wrapping all frame-relevant data into a single object
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		camera& cameraInstance;
	};

	class rendersystem {
	public:
		rendersystem(device& deviceInstance, VkRenderPass renderPass); // constructor
		~rendersystem(); // destructor

		// not copyable or movable
		rendersystem(const rendersystem&) = delete;
		rendersystem& operator = (const rendersystem&) = delete;

		void renderEntities(FrameInfo& frameInfo, std::vector<entity>& entities); // render the entities

	private:
		void createPipelineLayout(); // create a pipeline layout
		void createPipeline(VkRenderPass renderPass); // create a pipeline
		
		device& deviceInstance; // a handle for the device instance
		std::unique_ptr<pipeline> pipelineInstance; // a handle for the pipeline instance
		VkPipelineLayout pipelineLayout; // a handle for the pipeline layout
	};
}