#pragma once
#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace ToyBox {
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

	// struct for point lights
	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	class Entity {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, Entity>;

		// create the entity
		static Entity createEntity() {
			static id_t currentId = 0;
			return Entity{ currentId++ };
		}

		// not copyable or movable
		Entity(const Entity&) = delete;
		Entity& operator = (const Entity&) = delete;
		Entity(Entity&&) = default;
		Entity& operator = (Entity&&) = default;

		static Entity makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		id_t getId() { return id; } // return the entity id

		std::shared_ptr<Model> model = {};
		glm::vec3 color = {};
		TransformComponent transform = {};

		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		Entity(id_t entityId) : id{entityId} {} // constructor

		id_t id;
	};
}