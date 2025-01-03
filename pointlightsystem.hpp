#pragma once
#include "camera.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "frameinfo.hpp"
#include <memory>
#include <vector>

namespace engine {
	class pointlightsystem {
	public:
		pointlightsystem(device& deviceInstance, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout); // constructor
		~pointlightsystem(); // destructor

		// not copyable or movable
		pointlightsystem(const pointlightsystem&) = delete;
		pointlightsystem& operator = (const pointlightsystem&) = delete;

		void render(FrameInfo& frameInfo); // render the entities

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout); // create a pipeline layout
		void createPipeline(VkRenderPass renderPass); // create a pipeline

		device& deviceInstance; // a handle for the device instance
		std::unique_ptr<pipeline> pipelineInstance; // a handle for the pipeline instance
		VkPipelineLayout pipelineLayout; // a handle for the pipeline layout
	};
}