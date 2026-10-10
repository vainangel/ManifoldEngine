#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in int aTex;

out vec2 TexCoord;
flat out ivec4 color; 
flat out int c;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//out vec3 world_pos; 
//out vec3 world_normal;
//out vec4 viewSpace;

void main() {
	//world_pos = (model * vec4(aPos, 1)).xyz;
	//world_normal = normalize(mat3(model) * aNormal);
	TexCoord = vec2(aTex);

	//color = ivec4(aTex & 0xFF, (aTex >> 8) & 0xFF, (aTex >> 16) & 0xFF, (aTex >> 24) & 0xFF);
	color = ivec4(1);
	c = aTex; 

	vec4 viewSpace = view * model * vec4(aPos, 1.0f);
	gl_Position = projection * viewSpace;
}
