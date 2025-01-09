#pragma once
#include "camera.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "frameinfo.hpp"
#include <memory>
#include <vector>

namespace ToyBox {
	class PointLightSystem {
	public:
		PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout); // constructor
		~PointLightSystem(); // destructor

		// not copyable or movable
		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator = (const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo); // update the point light array
		void render(FrameInfo& frameInfo); // render the entities

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout); // create a pipeline layout
		void createPipeline(VkRenderPass renderPass); // create a pipeline

		Device& device; // a handle for the device instance
		std::unique_ptr<Pipeline> pipeline; // a handle for the pipeline instance
		VkPipelineLayout pipelineLayout; // a handle for the pipeline layout
	};
}