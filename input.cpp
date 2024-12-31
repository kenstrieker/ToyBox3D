#include "input.hpp"
#include <limits>

namespace engine {
	void input::moveInPlaneXZ(GLFWwindow* windowInstance, float dt, entity& entityInstance) {
		glm::vec3 rotate{ 0 };
		if (glfwGetKey(windowInstance, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(windowInstance, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(windowInstance, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(windowInstance, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) { // to avoid normalizing zero
			entityInstance.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// limit pitch values between about +/- 85 degrees
		entityInstance.transform.rotation.x = glm::clamp(entityInstance.transform.rotation.x, -1.5f, 1.5f);
		entityInstance.transform.rotation.y = glm::mod(entityInstance.transform.rotation.y, glm::two_pi<float>());

		float yaw = entityInstance.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };
		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(windowInstance, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(windowInstance, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(windowInstance, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(windowInstance, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(windowInstance, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(windowInstance, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) { // to avoid normalizing zero
			entityInstance.transform.translation += lookSpeed * dt * glm::normalize(moveDir);
		}
	}
}