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
		pipeline(device& deviceInstance, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
		~pipeline();

		pipeline(const pipeline&) = delete;
		void operator = (const pipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

	private:
		static std::vector<char> readFile(const std::string& filepath);
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		device& deviceInstance;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}