#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace engine {
	class camera {
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far); // to define each plane of the orthographic viewing volume
		void setPerspectiveProjection(float fovy, float aspect, float near, float far); // to define the field of view of the perspective projection

		// setters
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }); // last arg is the direction of up (in the direction of the -y axis)
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }); // useful for a camera locked onto a specific point in space
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation); // use euler angles to set the direction of the camera

		// getters
		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }

	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
	};
}