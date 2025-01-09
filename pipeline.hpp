#pragma once
#include "device.hpp"
#include <string>
#include <vector>

namespace ToyBox {
	// struct to contain and share data on how we want to configure the pipeline
	struct PipelineConfigInfo {
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator = (const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline {
	public:
		Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo); // constructor
		~Pipeline(); // destructor

		// not copyable or movable
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator = (const Pipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer); // bind a pipeline
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo); // to set up the pipeline's fixed functions

	private:
		static std::vector<char> readFile(const std::string& filepath); // to read a file
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo); // to set up the graphics pipeline
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule); // for loading vertex buffer data

		Device& device; // reference to device; this will outlive any instances of this class as a pipeline depends on a device to exist
		VkPipeline graphicsPipeline; // a handle to the graphics pipeline
		VkShaderModule vertShaderModule; // a handle to the vertex shader
		VkShaderModule fragShaderModule; // a handle to the fragment shader
	};
}