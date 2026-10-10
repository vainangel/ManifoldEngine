#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <string_view>
#include <optional>
#include <string>
#include <vector>

#include "Shader.hpp"

namespace mk {
	struct ShaderSource {
		GLuint shader_type;
		std::string shader_resource_path;
	};

	/// <summary>
	/// A context for a shader program. Does not automatically destroy the managed shader if it's shared pointer
	/// is held in another context. 
	/// </summary>
	class ShaderHandle {
	private:
		std::shared_ptr<Shader> m_shader = nullptr;
		std::vector<ShaderSource> m_sources;
	public:
		ShaderHandle(std::initializer_list<ShaderSource> shader_sources);
	
		std::shared_ptr<Shader> get();
	};

	/// <summary>
	/// A shader context manager to automate the initialization, reloading, and destruction of shader handles.
	/// </summary>
	class ShaderManager {
	private:
		std::unordered_map<std::string, ShaderHandle> m_shaders_by_name;
	public:
		ShaderManager& add_shader(std::string_view name, ShaderHandle);
		std::optional<std::shared_ptr<Shader>> get_shader(std::string_view name);
	};
}
