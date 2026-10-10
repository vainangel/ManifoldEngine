#version 450 core
layout (location = 0) in uvec3 aPos;
layout (location = 1) in int textureID;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out flat uvec3 normal;
out vec2 texCoord; 
out vec3 thisVertexCoord;
out flat int texID;
out float v_fogDepth;
out float v_flatDepth;
out vec3 FragPos;

const uvec3 n_front =	uvec3(0, 0, 1);
const uvec3 n_back =	uvec3(0, 1, 0);
const uvec3 n_left =	uvec3(1, 0, 1);
const uvec3 n_right =	uvec3(1, 0, 0);
const uvec3 n_top =		uvec3(1, 1, 1);
const uvec3 n_bottom =	uvec3(0, 0, 0);

void main() {
	normal = uvec3(aPos.x & 128, aPos.y & 128, aPos.z & 128);
	uvec3 n = uvec3(normal.x >> 7, normal.y >> 7, normal.z >> 7);

	// Extract the bottom 7 bits of each vertex to read the 3D vertex coordinate, parse as f32[3]
	vec3 fPos = vec3(aPos.x & 127, aPos.y & 127, aPos.z & 127);

	// ? generate texture coords from vertex coords and normal map 
	// 
	// texCoord is not normalized. E.g., when fPos.xz is (40.00, 45.00), texCoord is (40.00, 45.00). 
	if (n == n_front || n == n_back) {
		texCoord = fPos.xy;
	} else if (n == n_left || n == n_right) {
		texCoord = fPos.yz;
	} else if (n == n_top || n == n_bottom) {
		texCoord = fPos.xz;
	}

	texCoord += textureID;
	thisVertexCoord = fPos;
	texID = textureID;

	vec4 viewSpace = view * model * vec4(fPos, 1.0f);
	vec4 clipPos = projection * viewSpace;
	gl_Position = clipPos;
	v_fogDepth = -viewSpace.z;

	// im very lost in the rounding sauce here 
	vec4 clampedModelSpace = model * ceil(vec4(fPos, 1.0f));
	clampedModelSpace.xyz = floor(clampedModelSpace.xyz);
	vec4 clampedViewSpace = view * clampedModelSpace;
	v_flatDepth = length(clampedViewSpace.xyz);
	FragPos = vec3(view * model * (vec4(fPos, 1.0f)));
}

// 	//vec3 fPosClamp = clamp(normalize(fPos), 0.0f, 1.0f);
	//vec3 fPosClamp = vec3(fPos.x); // does not seem to do anything of note 
	