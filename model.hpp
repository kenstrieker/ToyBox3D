#pragma once
#include "device.hpp"
#include "buffer.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace ToyBox {
	class Model {
	public:
		// struct for vertex attributes to make them easier to work with
		struct Vertex {
			glm::vec3 position = {};
			glm::vec3 color = {};
			glm::vec3 normal = {};
			glm::vec2 uv = {};
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		// struct for holding vertex and index information until it can be copied into the model's buffer memory
		struct Builder {
			std::vector<Vertex> vertices = {};
			std::vector<uint32_t> indices = {};
			void loadModel(const std::string& filepath);
		};

		Model(Device& device, const Model::Builder& builder); // constructor
		~Model(); // destructor

		// not copyable or movable
		Model(const Model&) = delete;
		Model& operator = (const Model&) = delete;

		static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices); // to create the vertex buffers
		void createIndexBuffer(const std::vector<uint32_t>& indices); // to create the index buffers
		Device& device; // reference to the device

		std::unique_ptr<Buffer> vertexBuffer; // a handle for the vertex buffer
		uint32_t vertexCount; // a handle for the count of vertices
		bool hasIndexBuffer = false; // a flag for using index buffers
		std::unique_ptr<Buffer> indexBuffer; // a handle for the index buffer
		uint32_t indexCount; // a handle for the count of indices
	};
}