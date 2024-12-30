#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace engine {
	class camera {
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far); // to define each plane of the orthographic viewing volume
		void setPerspectiveProjection(float fovy, float aspect, float near, float far); // to define the field of view of the perspective projection
		const glm::mat4& getProjection() const { return projectionMatrix; }

	private:
		glm::mat4 projectionMatrix{ 1.f };
	};
}