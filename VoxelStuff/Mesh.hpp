#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <memory>

#include "Transform.hpp"
#include "Shader.hpp"

namespace mk {
	struct Vertex {
		glm::vec3 pos;
		int color; 
	};

	// Normal specifier baked into leading bit of each vertex. 
	// [nxxxxxxx][nyyyyyyy][nzzzzzzz][tttttttt]

	struct u8vec3 { uint8_t x, y, z; };

	struct VoxelMeshVertex {
		u8vec3 pos;
		uint8_t texture_id;
	};

	struct VoxelMesh;

	struct VoxelMeshGLRenderHandle {
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;

		GLuint ebo_length = 0;
	
		VoxelMeshGLRenderHandle() = default;
		VoxelMeshGLRenderHandle(const VoxelMesh&);

		~VoxelMeshGLRenderHandle(); // does nothing until heap-allocated sources are used to prevent premature deallocation
	};

	struct VoxelMesh {
		std::vector<VoxelMeshVertex> vertices;
		std::vector<GLuint> indices;
		VoxelMeshGLRenderHandle gl_handle;
	};

	struct VoxelChunkObject {
		mk::VoxelMesh mesh;
		mk::Transform transform;
		std::shared_ptr<mk::Shader> shader;
	};

	struct Quadface {
		mk::Vertex top_left;
		mk::Vertex top_right;
		mk::Vertex bottom_left;
		mk::Vertex bottom_right;
	};

	struct Mesh;

	struct GLMeshHandle {
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		GLuint ssbo;

		GLuint indices;

		/// <summary>
		/// Initializes a GPU mesh from a Mesh.
		/// </summary>
		/// <param name="mesh"></param>
		/// <returns>What do you think this returns?</returns>
		static GLMeshHandle from_mesh(const Mesh& mesh);

		void update_mesh(const Mesh& mesh);

		static GLMeshHandle get_sample_quad();
		static GLMeshHandle get_sample_cube();
	};

	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		GLMeshHandle m_gl_mesh_handle = {};
		std::shared_ptr<Shader> m_shader_used; 

		GLuint cull_face = GL_BACK;
		bool depth_mask = true;

		bool mesh_changed = false;

		static Mesh* get_sample_cube();

		Mesh* with_shader(std::shared_ptr<Shader> shader);

		/// <summary>
		/// Initialize the internal GL mesh handle. 
		/// </summary>
		void init_gl_handle();

		/// <summary>
		/// Clears the mesh buffers. Should only be called after initializing the GL mesh handle. 
		/// </summary>
		void clear();
	};

	std::shared_ptr<mk::Mesh> create_ssbo_test_mesh(GLuint shader);
}
