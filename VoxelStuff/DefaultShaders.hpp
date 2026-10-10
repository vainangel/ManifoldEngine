#pragma once 

#include <string_view>

namespace mk {
	constexpr std::string_view vertex_shader_src = ""
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 normalPos;\n"
		"layout (location = 2) in vec2 uvPos;\n"
		"\n"
		"void main() {\n"
		"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\n";

	constexpr std::string_view fragment_shader_src = ""
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"\n"
		"void main() {\n"
		"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n";
}
