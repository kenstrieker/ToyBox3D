#include "model.hpp"
#include <cassert>

namespace engine {
	model::model(device& deviceInstance, const std::vector<Vertex>& vertices) : deviceInstance{ deviceInstance } {
		createVertexBuffers(vertices);
	}

	model::~model() {
		vkDestroyBuffer(deviceInstance.getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(deviceInstance.getDevice(), vertexBufferMemory, nullptr);
	}

	void model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		// check that we have at least one triangle (3 vertices)
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		// create a vertex buffer
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		deviceInstance.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
		
		// create a region of host memory mapped to device memory and set data to point to the beginning of the mapped memory range
		void* data;
		vkMapMemory(deviceInstance.getDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceInstance.getDevice(), vertexBufferMemory);
	}

	void model::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void model::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> model::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> model::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
}