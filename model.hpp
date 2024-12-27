#pragma once
#include "device.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace engine {
	class model {
	public:
		// struct for vertex attributes to make them easier to work with
		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		model(device& deviceInstance, const std::vector<Vertex>& vertices); // constructor
		~model(); // destructor

		// not copyable or movable
		model(const model&) = delete;
		model& operator = (const model&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices); // to create the vertex buffers
		device& deviceInstance; // reference to the device
		VkBuffer vertexBuffer; // a handle for the vertex buffer
		VkDeviceMemory vertexBufferMemory; // a handle for the memory containing the vertex buffer
		uint32_t vertexCount; // a handle for the count of vertices
	};
}