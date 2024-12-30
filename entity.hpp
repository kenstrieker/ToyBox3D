#pragma once
#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace engine {
	// struct for translating
	struct TransformComponent {
		glm::vec3 translation = {}; // position offset
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation = {};

		// matrix corresponds to translate * ry * rx * rz * scale transformation
		// using tait-bryan angles for rotation convention with y(1), x(2), z(3) axis order
		glm::mat4 mat4() { // 3 spatial dimensions, plus one more for homogeneous coordinates
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{ scale.x * (c1 * c3 + s1 * s2 * s3), scale.x * (c2 * s3), scale.x * (c1 * s2 * s3 - c3 * s1), 0.0f, },
				{ scale.y * (c3 * s1 * s2 - c1 * s3), scale.y * (c2 * c3), scale.y * (c1 * c3 * s2 + s1 * s3), 0.0f, },
				{ scale.z * (c2 * s1), scale.z * (-s2),	scale.z * (c1 * c2), 0.0f, },
				{ translation.x, translation.y, translation.z, 1.0f} 
			};
		}
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
		TransformComponent transform = {};

	private:
		entity(id_t entityId) : id{entityId} {} // constructor

		id_t id;
	};
}