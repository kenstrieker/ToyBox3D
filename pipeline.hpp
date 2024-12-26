#pragma once
#include "device.hpp"
#include <string>
#include <vector>

namespace engine {
	// struct to contain and share data on how we want to configure the pipeline
	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class pipeline {
	public:
		pipeline(device& deviceInstance, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo); // constructor
		~pipeline(); // destructor

		// not copyable or movable
		pipeline(const pipeline&) = delete;
		void operator = (const pipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer); // bind a pipeline
		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height); // to set up the pipeline's fixed functions

	private:
		static std::vector<char> readFile(const std::string& filepath); // to read a file
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo); // to set up the graphics pipeline
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule); // for loading vertex buffer data

		device& deviceInstance; // reference to device; this will outlive any instances of this class as a pipeline depends on a device to exist
		VkPipeline graphicsPipeline; // a handle to the graphics pipeline
		VkShaderModule vertShaderModule; // a handle to the vertex shader
		VkShaderModule fragShaderModule; // a handle to the fragment shader
	};
}