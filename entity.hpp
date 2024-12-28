#pragma once
#include "model.hpp"
#include <memory>

namespace engine {
	// struct for translating up, down, left, and right
	struct Transform2DComponent {
		glm::vec2 translation = {};
		glm::vec2 scale{ 1.f, 1.f };
		float rotation;

		glm::mat2 mat2() {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c,s}, {-s, c} };
			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };
			return rotMatrix * scaleMat;
		}; 
	};

	class entity {
	public:
		using id_t = unsigned int;

		// create the entity
		static entity createEntity() {
			static id_t currentId = 0;
			return entity{ currentId++ };
		}

		// not copyable or movable
		entity(const entity&) = delete;
		entity& operator = (const entity&) = delete;
		entity(entity&&) = default;
		entity& operator = (entity&&) = default;

		id_t getId() { return id; } // return the entity id

		std::shared_ptr<model> modelInstance = {};
		glm::vec3 color = {};
		Transform2DComponent transform2d = {};

	private:
		entity(id_t entityId) : id{entityId} {} // constructor

		id_t id;
	};
}