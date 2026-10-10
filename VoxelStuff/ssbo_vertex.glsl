#version 450 core 

// Our SSBO added in step 4
layout(std430, binding = 0) readonly buffer vertexPullBuffer 
{
	uint packedMeshData[]; // Contains packed data for our vertices
};

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Offsets for our vertices drawing this face
const vec3 facePositions[4] = vec3[4]
(
	vec3(0.0f, 0.0f, 1.0f),
	vec3(1.0f, 0.0f, 0.0f),
	vec3(1.0f, 0.0f, 1.0f),
	vec3(0.0f, 0.0f, 0.0f)
);

// Winding order to access the face positions
int indices[6] = {0, 1, 2, 1, 0, 3};

void main()
{
	// Create a custom index to access the mesh data in the right order
	const int index = gl_VertexID / 6;
	const uint packedData = packedMeshData[index];
	const int currVertexID = gl_VertexID % 6;

	// Unpack the data we retrieved
	const uint x = (packedData) & 1023;
	const uint y = (packedData >> 10) & 1023;
	const uint z = (packedData >> 20) & 1023;
	vec3 position = vec3(x, y, z);
  
	// Apply the offsets for our face so we can form the 2 triangles
	position += facePositions[indices[currVertexID]];

	// Output our position
	//gl_Position = vec4(position, 1.0);

	vec4 viewSpace = view * model * vec4(position, 1.0f);
	gl_Position = projection * viewSpace;
}