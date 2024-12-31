#include "model.hpp"
#include <cassert>

namespace engine {
	model::model(device& deviceInstance, const model::Builder& builderInstance) : deviceInstance{ deviceInstance } {
		createVertexBuffers(builderInstance.vertices);
		createIndexBuffer(builderInstance.indices);
	}

	model::~model() {
		vkDestroyBuffer(deviceInstance.getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(deviceInstance.getDevice(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(deviceInstance.getDevice(), indexBuffer, nullptr);
			vkFreeMemory(deviceInstance.getDevice(), indexBufferMemory, nullptr);
		}
	}

	void model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		// check that we have at least one triangle (3 vertices)
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		// create a staging buffer
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		deviceInstance.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		// map the staging buffer memory
		void* data;
		vkMapMemory(deviceInstance.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceInstance.getDevice(), stagingBufferMemory);

		// create a vertex buffer
		deviceInstance.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		deviceInstance.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(deviceInstance.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(deviceInstance.getDevice(), stagingBufferMemory, nullptr);
	}

	void model::createIndexBuffer(const std::vector<uint32_t>& indices) {
		// check that we are using an index buffer for rendering
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer) return;

		// create a staging buffer
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		deviceInstance.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// map the staging buffer memory
		void* data;
		vkMapMemory(deviceInstance.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceInstance.getDevice(), stagingBufferMemory);

		// create a vertex buffer
		deviceInstance.createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
		deviceInstance.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(deviceInstance.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(deviceInstance.getDevice(), stagingBufferMemory, nullptr);
	}

	void model::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void model::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
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
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
}