#include "Shader.hpp"

#include <iostream>

mk::Shader* mk::Shader::create_shader() {
	Shader* shader = new Shader;
	shader->program_id = glCreateProgram();

	return shader;
}

// want to replace with an idiomatic result type of some sort 
template <size_t InfoLogLength> 
static void check_shader_compile_status(GLuint shader, int* success, char(*info_log)[InfoLogLength]) {
	glGetShaderiv(shader, GL_COMPILE_STATUS, success);
	if (!*success) {
		glGetShaderInfoLog(shader, InfoLogLength, nullptr, *info_log);
	}
}

template <size_t InfoLogLength>
static void check_program_link_status(GLuint program, int* success, char(*info_log)[InfoLogLength]) {
	glGetProgramiv(program, GL_LINK_STATUS, success);
	if (!*success) {
		glGetProgramInfoLog(program, InfoLogLength, nullptr, *info_log);
	}
}

mk::Shader* mk::Shader::with_vertex(std::string_view src) {
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	const char* data = src.data(); // i hope this isnt UB 
	glShaderSource(vertex_shader, 1, &data, nullptr);
	glCompileShader(vertex_shader);

	int success;
	char info_log[512];
	check_shader_compile_status<512>(vertex_shader, &success, &info_log);

	if (!success) {
		std::cerr << "Failed to compile vertex shader: " << info_log << '\n';
	}
	else {
		glAttachShader(program_id, vertex_shader);
	}

	return this;
}

mk::Shader* mk::Shader::with_fragment(std::string_view src) {
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* data = src.data();
	glShaderSource(fragment_shader, 1, &data, nullptr);
	glCompileShader(fragment_shader);

	int success;
	char info_log[512];
	check_shader_compile_status<512>(fragment_shader, &success, &info_log);

	if (!success) {
		std::cerr << "Failed to compile fragment shader: " << info_log << '\n';
	}
	else {
		glAttachShader(program_id, fragment_shader);
	}

	return this;
}

mk::Shader* mk::Shader::with_geometry(std::string_view src) {
	geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);

	const char* data = src.data();
	glShaderSource(geometry_shader, 1, &data, nullptr);
	glCompileShader(geometry_shader);

	int success;
	char info_log[512];
	check_shader_compile_status<512>(geometry_shader, &success, &info_log);

	if (!success) {
		std::cerr << "Failed to compile geometry shader: " << info_log << '\n';
	}
	else {
		glAttachShader(program_id, geometry_shader);
	}

	return this;
}

mk::Shader* mk::Shader::link() {
	glLinkProgram(program_id);

	int success;
	char info_log[512]; // i couldve passed by reference dude lol 
	check_program_link_status<512>(program_id, &success, &info_log);

	if (!success) {
		std::cerr << "Failed to link program: " << info_log << '\n';
	}

	return this;
}

void mk::Shader::bind() const {
	glUseProgram(program_id);
}

mk::Shader::~Shader() {
	glDeleteProgram(program_id);
}

GLuint mk::Shader::id() const { return program_id; }

GLuint mk::Shader::get_uniform_location(std::string name) {
	auto&& uloc = m_uniform_cache.find(name);
	if (uloc == m_uniform_cache.end()) {
		GLuint retval = glGetUniformLocation(id(), name.c_str());
		m_uniform_cache.insert({ name, retval });
		return retval;
	}

	return uloc->second; 
}