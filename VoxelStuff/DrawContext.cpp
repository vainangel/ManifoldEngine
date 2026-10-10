#include "DrawContext.hpp"

#include <iostream> // replace with logging eventually 
#include <print>
#include <string_view>
#include <string>

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

static bool firstMouse = true; 

static mk::DrawContext* global_draw_ctx = nullptr;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		if (global_draw_ctx->toggle_wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		global_draw_ctx->toggle_wireframe = !global_draw_ctx->toggle_wireframe;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		if (global_draw_ctx->toggle_cull_front_face) {
			glEnable(GL_CULL_FACE);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		global_draw_ctx->toggle_cull_front_face = !global_draw_ctx->toggle_cull_front_face;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		global_draw_ctx->remesh = true;
	}

	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		global_draw_ctx->remesh = true;
		global_draw_ctx->use_legacy = !global_draw_ctx->use_legacy;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		global_draw_ctx->regenerate = true;
		global_draw_ctx->gen_params.scale -= 0.001;
		global_draw_ctx->remesh = true;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		global_draw_ctx->regenerate = true;
		global_draw_ctx->gen_params.scale += 0.001;
		global_draw_ctx->remesh = true;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		global_draw_ctx->regenerate = true;
		global_draw_ctx->gen_params.offset_x += 1;
		global_draw_ctx->remesh = true;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		global_draw_ctx->regenerate = true;
		global_draw_ctx->gen_params.offset_x -= 1;
		global_draw_ctx->remesh = true;
	}
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		global_draw_ctx->camera().lastX = xpos;
		global_draw_ctx->camera().lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - global_draw_ctx->camera().lastX;
	float yoffset = global_draw_ctx->camera().lastY - ypos;
	global_draw_ctx->camera().lastX = xpos;
	global_draw_ctx->camera().lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	global_draw_ctx->camera().yaw += xoffset;
	global_draw_ctx->camera().pitch += yoffset;

	if (global_draw_ctx->camera().pitch > 89.0f)
		global_draw_ctx->camera().pitch = 89.0f;
	if (global_draw_ctx->camera().pitch < -89.0f)
		global_draw_ctx->camera().pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(global_draw_ctx->camera().yaw)) * cos(glm::radians(global_draw_ctx->camera().pitch));
	direction.y = sin(glm::radians(global_draw_ctx->camera().pitch));
	direction.z = sin(glm::radians(global_draw_ctx->camera().yaw)) * cos(glm::radians(global_draw_ctx->camera().pitch));
	global_draw_ctx->camera().front = glm::normalize(direction);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_1) {
			
		}
	}
}

mk::DrawContext* mk::DrawContext::create_context(int width, int height, std::string_view title) {
	static DrawContext* ctx = new DrawContext;
	ctx->clear_color = { 0, 0.5, 0.5, 1.0 };
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_SAMPLES, 32);

	ctx->window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!ctx->window) {
		std::println(stderr, "Failed to initialize GLFW WINDOW.");
		std::exit(3);
	}
	ctx->bind();

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0) {
		std::println(stderr, "Failed to initialize OpenGL.");
		std::exit(4);
	}

	const unsigned char* s = glGetString(GL_VERSION);
	std::println("{}", std::string(reinterpret_cast<const char*>(s)));

	std::println("Initialized OpenGL and window.");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);

	auto w_sz = ctx->window_size();
	glViewport(0, 0, w_sz.x, w_sz.y);

	glfwSetFramebufferSizeCallback(ctx->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(ctx->window, mouse_callback);
	glfwSetKeyCallback(ctx->window, key_callback);
	glfwSetMouseButtonCallback(ctx->window, mouse_button_callback);
	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(ctx->window);
	glfwSwapInterval(0);

	global_draw_ctx = ctx;
	ctx->m_camera.lastX = width / 2;
	ctx->m_camera.lastY = height / 2;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	int windowWidth, windowHeight;
	glfwGetWindowSize(ctx->window, &windowWidth, &windowHeight);

	int x = (mode->width - windowWidth) / 2;
	int y = (mode->height - windowHeight) / 2;

	glfwSetWindowPos(ctx->window, x, y);

	return ctx;
}

bool mk::DrawContext::should_close() const {
	return glfwWindowShouldClose(window);
}

void mk::DrawContext::bind() {
	glfwMakeContextCurrent(window);
}

void mk::DrawContext::draw() {
	glfwMakeContextCurrent(window);

	process_input();

	glDepthMask(true); // TODO: Replace depth mask toggle with the use of depth functions for each object mesh. 
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto&& w_sz = window_size();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)w_sz.x / w_sz.y, near, far);
	glm::mat4 view = glm::identity<glm::mat4>();
	view = glm::lookAt(camera().pos, camera().pos + camera().front, camera().up);

	// pop draw queue
	while (not draw_queue.empty()) {
		auto&& object = draw_queue.front();
	
		if (object->mesh()->m_gl_mesh_handle.vao == 0) {
			object->mesh()->init_gl_handle();
		}

		auto&& shader = object->mesh()->m_shader_used;
		shader->bind();

		glm::mat4 model = glm::identity<glm::mat4>();
		model = glm::translate(model, object->transform().position);
		model = glm::rotate(model, object->transform().pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, object->transform().yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, object->transform().roll, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(object->transform().scale));

		GLuint u_projection = shader->get_uniform_location("projection");
		GLuint u_view = shader->get_uniform_location("view");
		GLuint u_model = shader->get_uniform_location("model");

		if (u_projection != GL_INVALID_VALUE) glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection));
		if (u_view != GL_INVALID_VALUE) glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
		if (u_model != GL_INVALID_VALUE) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

		glCullFace(object->mesh()->cull_face);
		glDepthMask(object->mesh()->depth_mask);
		
		glBindVertexArray(object->mesh()->m_gl_mesh_handle.vao);
		if (object->mesh()->m_gl_mesh_handle.ebo != 0)
			glDrawElements(GL_TRIANGLES, object->mesh()->m_gl_mesh_handle.indices, GL_UNSIGNED_INT, 0);
		else if (object->mesh()->m_gl_mesh_handle.ssbo != 0)
			glDrawArrays(GL_TRIANGLES, 0, object->mesh()->indices.size() * 12);

		glBindVertexArray(0);

		draw_queue.pop(); // i genuinely think drawing nothing after the skybox just makes it somehow fail the depth test... even though it isnt depth-tested?
	}

	glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);

	while (not voxel_draw_queue.empty()) {
		auto&& object = voxel_draw_queue.front();

		if (object->mesh.gl_handle.vao == 0) {
			//std::println("Creating gl handle for this chunk...");
			object->mesh.gl_handle = mk::VoxelMeshGLRenderHandle(object->mesh);
		}

		auto&& shader = object->shader;
		shader->bind();

		glm::mat4 model = glm::identity<glm::mat4>();
		model = glm::translate(model, object->transform.position);
		model = glm::rotate(model, object->transform.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, object->transform.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, object->transform.roll, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(object->transform.scale));

		GLuint u_projection = shader->get_uniform_location("projection");
		GLuint u_view = shader->get_uniform_location("view");
		GLuint u_model = shader->get_uniform_location("model");

		if (u_projection != GL_INVALID_VALUE) glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection));
		if (u_view != GL_INVALID_VALUE) glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
		if (u_model != GL_INVALID_VALUE) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(object->mesh.gl_handle.vao);
		glDrawElements(GL_TRIANGLES, object->mesh.gl_handle.ebo_length, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		voxel_draw_queue.pop();
	}

	/*glfwSwapBuffers(window);
	glfwPollEvents();*/
}

void mk::DrawContext::draw_world_object_immediately(mk::WorldObject* object) {
	auto&& w_sz = window_size();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)w_sz.x / w_sz.y, near, far);
	glm::mat4 view = glm::identity<glm::mat4>();
	view = glm::lookAt(camera().pos, camera().pos + camera().front, camera().up);

	if (object->mesh()->m_gl_mesh_handle.vao == 0) {
		object->mesh()->init_gl_handle();
	}

	auto&& shader = object->mesh()->m_shader_used;
	shader->bind();

	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, object->transform().position);
	model = glm::rotate(model, object->transform().pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, object->transform().yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, object->transform().roll, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(object->transform().scale));

	GLuint u_projection = shader->get_uniform_location("projection");
	GLuint u_view = shader->get_uniform_location("view");
	GLuint u_model = shader->get_uniform_location("model");

	if (u_projection != GL_INVALID_VALUE) glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection));
	if (u_view != GL_INVALID_VALUE) glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	if (u_model != GL_INVALID_VALUE) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

	glCullFace(object->mesh()->cull_face);
	glDepthMask(object->mesh()->depth_mask);

	glBindVertexArray(object->mesh()->m_gl_mesh_handle.vao);
	if (object->mesh()->m_gl_mesh_handle.ebo != 0)
		glDrawElements(GL_TRIANGLES, object->mesh()->m_gl_mesh_handle.indices, GL_UNSIGNED_INT, 0);
	else if (object->mesh()->m_gl_mesh_handle.ssbo != 0)
		glDrawArrays(GL_TRIANGLES, 0, object->mesh()->indices.size() * 12);

	glBindVertexArray(0);
}

glm::ivec2 mk::DrawContext::window_size() const {
	glm::ivec2 retval = {};
	glfwGetWindowSize(window, &retval.x, &retval.y);

	return retval;
}

void mk::DrawContext::process_input() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	static float deltaTime = 0.0f;	// Time between current frame and last frame
	static float lastFrame = 0.0f; // Time of last frame

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	float cameraSpeedMult = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 50.0f : 10.0f;
	cameraSpeedMult *= glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 3.0f : 1.0f;
	float cameraSpeed = cameraSpeedMult * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera.pos += cameraSpeed * m_camera.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera.pos -= cameraSpeed * m_camera.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera.pos -= glm::normalize(glm::cross(m_camera.front, m_camera.up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera.pos += glm::normalize(glm::cross(m_camera.front, m_camera.up)) * cameraSpeed;
}

void mk::DrawContext::schedule_mesh_to_draw(mk::WorldObject* object) {
	draw_queue.push(object);
}

mk::Camera& mk::DrawContext::camera() { return m_camera; }
