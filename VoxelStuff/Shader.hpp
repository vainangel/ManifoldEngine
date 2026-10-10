#pragma once 

#include <glad/gl.h>

#include <unordered_map>
#include <string_view>
#include <memory>
#include <string>

namespace mk {
	//enum class ShaderType {
	//	Vertex,
	//	Fragment,
	//	Tessellation,
	//	Geometry,
	//	Compute
	//};

	//struct VertexShader {
	//	constexpr static ShaderType type = ShaderType::Vertex;
	//	GLuint id;
	//	static VertexShader from(std::string_view src);
	//};

	//struct FragmentShader {
	//	constexpr static ShaderType type = ShaderType::Fragment;
	//	GLuint id;
	//	static FragmentShader from(std::string_view src);
	//};

	//struct TessellationShader {
	//	constexpr static ShaderType type = ShaderType::Tessellation;
	//	GLuint id;
	//	static TessellationShader from(std::string_view src);
	//};

	//struct GeometryShader {
	//	constexpr static ShaderType type = ShaderType::Geometry;
	//	GLuint id;
	//	static GeometryShader from(std::string_view src);
	//};

	//struct ComputeShader {
	//	constexpr static ShaderType type = ShaderType::Compute;
	//	GLuint id;
	//	static ComputeShader from(std::string_view src);
	//};

	//template <class T> concept ShaderStruct = requires (T t, std::string_view src) {
	//	{ t.from(src) } -> std::same_as<T>;
	//	{ t.id } -> std::same_as<GLuint>;
	//	{ T::type } -> std::same_as<ShaderType>;
	//};

	//class Shader {
	//private:
	//	GLuint id;
	//public:
	//	// ...
	//};
	//
	//class ShaderBuilder {
	//private:
	//	GLuint program_id = 0UL;
	//	
	//	GLuint shaders[5] = {};

	//public:
	//	static ShaderBuilder create();

	//	template <ShaderStruct T> ShaderBuilder attach(T shader) {
	//		if (shaders[static_cast<size_t>(shader.type)]) {
	//			glDeleteShader(shaders[static_cast<size_t>(shader.type)]);
	//		}

	//		shaders[static_cast<size_t>(shader.type)] = shader;
	//	}

	//	std::shared_ptr<Shader> build();
	//};
	/**
	* @brief A shader program.
	*/
	class Shader {
	private:
		GLuint program_id;

		GLuint vertex_shader = 0UL;
		GLuint fragment_shader = 0UL;
		GLuint tessellation_shader = 0UL;
		GLuint geometry_shader = 0UL;

		std::unordered_map<std::string, GLuint> m_uniform_cache; 
	public:
		static Shader* create_shader();

		~Shader();

		Shader* with_vertex(std::string_view src);

		Shader* with_fragment(std::string_view src);

		Shader* with_tessellation(std::string_view src);

		Shader* with_geometry(std::string_view src);

		Shader* link();

		GLuint id() const;

		/// <summary>
		///  Binds shader program to the current GL context.
		/// </summary>
		void bind() const;

		GLuint get_uniform_location(std::string name);
	};
}