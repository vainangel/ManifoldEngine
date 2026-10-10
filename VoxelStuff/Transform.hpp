#pragma once 

#include <glm/vec3.hpp>

namespace mk {

	struct Transform {
		glm::vec3 position;
		float pitch;
		float yaw;
		float roll;
		float scale = 1.0f;
	};

}