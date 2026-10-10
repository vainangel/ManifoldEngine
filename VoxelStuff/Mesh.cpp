#include "Mesh.hpp"

#include <iostream>

mk::Mesh* mk::Mesh::with_shader(std::shared_ptr<Shader> shader) {
	m_shader_used = shader;

	if (!shader || shader->id() == 0) {
		std::cerr << "Warning: uninitialized shader program specified for mesh.\n";
	}

	return this;
}

void mk::Mesh::init_gl_handle() {
	if (m_gl_mesh_handle.vao == 0) {
		m_gl_mesh_handle = GLMeshHandle::from_mesh(*this);
	}
}

void mk::Mesh::clear() {
	vertices.clear();
	indices.clear();
}

mk::Mesh* mk::Mesh::get_sample_cube() {
	Mesh* retval = new Mesh;

	retval->vertices = {
		mk::Vertex({-1,  1,  1}, 1), // TL 0
		mk::Vertex({ 1,  1,  1}, 1), // TR 1
		mk::Vertex({ 1, -1,  1}, 1), // BR 2
		mk::Vertex({-1, -1,  1}, 1), // BL 3
		
		mk::Vertex({-1,  1, -1}, 1), // TL 4
		mk::Vertex({ 1,  1, -1}, 1), // TR 5
		mk::Vertex({ 1, -1, -1}, 1), // BR 6
		mk::Vertex({-1, -1, -1}, 1), // BL 7
		
		mk::Vertex({-1,  1, -1}, 1), // TL 8
		mk::Vertex({-1,  1,  1}, 1), // TR 9
		mk::Vertex({-1, -1,  1}, 1), // BR 10 
		mk::Vertex({-1, -1, -1}, 1), // BL 11 
		
		mk::Vertex({ 1,  1, -1}, 1), // TL 12
		mk::Vertex({ 1,  1,  1}, 1), // TR 13
		mk::Vertex({ 1, -1,  1}, 1), // BR 14 
		mk::Vertex({ 1, -1, -1}, 1), // BL 15 
		
		mk::Vertex({-1,  1,  1}, 1), // 16
		mk::Vertex({-1,  1, -1}, 1), // 17
		mk::Vertex({ 1,  1, -1}, 1), // 18
		mk::Vertex({ 1,  1,  1}, 1), // 19
		
		mk::Vertex({-1, -1,  1}, 1), // 20
		mk::Vertex({-1, -1, -1}, 1), // 21
		mk::Vertex({ 1, -1, -1}, 1), // 22
		mk::Vertex({ 1, -1,  1}, 1), // 23
	};

	retval->indices = {
		3, 1, 0, 3, 2, 1,
		4, 5, 7, 5, 6, 7,
		11, 9, 8, 11, 10, 9,
		12, 13, 15, 13, 14, 15,
		18, 17, 16, 16, 19, 18,
		20, 21, 22, 22, 23, 20,
	};

	return retval;
}

mk::GLMeshHandle mk::GLMeshHandle::from_mesh(const mk::Mesh& mesh) {
	GLMeshHandle retval = {};

	glGenVertexArrays(1, &retval.vao);
	glGenBuffers(1, &retval.vbo);
	glGenBuffers(1, &retval.ebo);

	glBindVertexArray(retval.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, retval.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mk::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mk::Vertex), (void*)0);
	glVertexAttribIPointer(1, 1, GL_INT, sizeof(mk::Vertex), (void*)(sizeof(glm::vec3)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, retval.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	retval.indices = mesh.indices.size();

	return retval;
}

void mk::GLMeshHandle::update_mesh(const Mesh& mesh) {
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mk::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	indices = mesh.indices.size();
}

mk::GLMeshHandle mk::GLMeshHandle::get_sample_quad() {
	static mk::Mesh mesh = {
		.vertices = {
			mk::Vertex{{0.5f,  0.5f, 0.0f}},  // top right
			mk::Vertex{{0.5f, -0.5f, 0.0f}},  // bottom right
			mk::Vertex{{-0.5f, -0.5f, 0.0f}},  // bottom left
			mk::Vertex{{-0.5f,  0.5f, 0.0f}}   // top left 
		},
		.indices = {
			0, 1, 3,
			1, 2, 3
		}
	};

	static mk::GLMeshHandle handle = mk::GLMeshHandle::from_mesh(mesh);

	return handle;
}

static std::vector<float> cube_base = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

mk::GLMeshHandle mk::GLMeshHandle::get_sample_cube() {
	return from_mesh(*mk::Mesh::get_sample_cube());
}

std::shared_ptr<mk::Mesh> mk::create_ssbo_test_mesh(GLuint shader) {
	auto mesh = std::make_shared<mk::Mesh>();

	GLuint ssbo, vao;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &ssbo);

	glm::ivec3 pos = glm::ivec3{ 5, 5, 5 };
	GLuint vertex = (pos.x | pos.y << 10 | pos.z << 20);
	mesh->indices.push_back(vertex);

	glUseProgram(shader);

	//glNamedBufferStorage(ssbo, ssbo_verts.size() * sizeof(int), ssbo_verts.data(), NULL);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, ssbo_verts.size() * sizeof (int), ssbo_verts.data(), NULL);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_verts.size() * sizeof(int), ssbo_verts.data(), NULL);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->indices.size() * sizeof(GLuint), mesh->indices.data(), NULL);

	glUseProgram(0);

	mesh->m_gl_mesh_handle.vao = vao;
	mesh->m_gl_mesh_handle.ssbo = ssbo;

	return mesh;
}

mk::VoxelMeshGLRenderHandle::VoxelMeshGLRenderHandle(const mk::VoxelMesh& mesh) {
	glGenVertexArrays(1, &vao);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VoxelMeshVertex) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

	glVertexAttribIPointer(0, 3, GL_BYTE, sizeof(VoxelMeshVertex), (void*)0);
	glVertexAttribIPointer(1, 1, GL_BYTE, sizeof(VoxelMeshVertex), (void*)sizeof(mk::u8vec3));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	ebo_length = mesh.indices.size();

	//std::println("{}, {}, {}", vao, vbo, ebo);
}

mk::VoxelMeshGLRenderHandle::~VoxelMeshGLRenderHandle() {
	/*glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);*/
}
