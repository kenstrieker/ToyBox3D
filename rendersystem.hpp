#pragma once
#include "camera.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "frameinfo.hpp"
#include <memory>
#include <vector>

namespace ToyBox {
	class RenderSystem {
	public:
		RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout); // constructor
		~RenderSystem(); // destructor

		// not copyable or movable
		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator = (const RenderSystem&) = delete;

		void renderEntities(FrameInfo& frameInfo); // render the entities

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout); // create a pipeline layout
		void createPipeline(VkRenderPass renderPass); // create a pipeline
		
		Device& device; // a handle for the device instance
		std::unique_ptr<Pipeline> pipeline; // a handle for the pipeline instance
		VkPipelineLayout pipelineLayout; // a handle for the pipeline layout
	};
}