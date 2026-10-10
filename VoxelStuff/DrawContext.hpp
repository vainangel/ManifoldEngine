#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "WorldObject.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string_view>
#include <queue>

namespace mk {
	using Color = glm::vec4;

	struct Camera {
		glm::vec3 pos;
		//glm::vec3 rot;
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);

		float lastX, lastY;
		float pitch = 0.0f, yaw = 0.0f; 
	};

	class DrawContext {
	public:
		GLFWwindow* window;
		Color clear_color;
		Camera m_camera;
		std::queue<mk::WorldObject*> draw_queue;
		std::queue<mk::VoxelChunkObject*> voxel_draw_queue;

		float near = 0.1;
		float far = 1000.0f;

		bool toggle_wireframe = false;
		bool toggle_cull_front_face = false;
		bool remesh = false;
		bool regenerate = false;
		struct GenerationParameters {
			float scale = 0.01;
			float offset_x = 0;
			float offset_z = 0;
		};
		GenerationParameters gen_params;
		bool use_legacy = false;
	public:
		static DrawContext* create_context(int width, int height, std::string_view title);

		bool should_close() const;
		void bind();

		void draw();

		void draw_world_object_immediately(mk::WorldObject* world_object);

		glm::ivec2 window_size() const; 
	
		void process_input();

		void schedule_mesh_to_draw(WorldObject* object);

		Camera& camera();
	};
}
