#include "ShaderStuff.hpp"
#include "IOUtil.hpp"

mk::ShaderHandle::ShaderHandle(std::initializer_list<ShaderSource> sources) {
	std::shared_ptr<Shader> shader = std::shared_ptr<Shader>(mk::Shader::create_shader());

	for (auto&& source : sources) {
		auto src = mk::util::get_resource(source.shader_resource_path);

		if (source.shader_type == GL_VERTEX_SHADER)
			shader->with_vertex(src);
		else if (source.shader_type == GL_FRAGMENT_SHADER)
			shader->with_fragment(src);
	
		m_sources.push_back(source);
	}

	shader->link();
	m_shader = shader;
}

std::shared_ptr<mk::Shader> mk::ShaderHandle::get() {
	return m_shader;
}

mk::ShaderManager& mk::ShaderManager::add_shader(std::string_view name, ShaderHandle handle) {
	m_shaders_by_name.insert({ std::string{name}, handle });

	return *this;
}

std::optional<std::shared_ptr<mk::Shader>> mk::ShaderManager::get_shader(std::string_view name) {
	auto match = m_shaders_by_name.find(std::string{name});

	if (match == m_shaders_by_name.end()) return std::nullopt;

	return match->second.get();
}