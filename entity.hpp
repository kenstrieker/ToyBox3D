#pragma once
#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace engine {
	// struct for translating
	struct TransformComponent {
		glm::vec3 translation = {}; // position offset
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation = {};

		// matrix corresponds to translate * ry * rx * rz * scale transformation
		// using tait-bryan angles for rotation convention with y(1), x(2), z(3) axis order
		glm::mat4 mat4();

		glm::mat3 normalMatrix();
	};

	class entity {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, entity>;

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
		TransformComponent transform = {};

	private:
		entity(id_t entityId) : id{entityId} {} // constructor

		id_t id;
	};
}