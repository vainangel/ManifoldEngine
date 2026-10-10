#include "DrawContext.hpp"
#include "Shader.hpp"
#include "DefaultShaders.hpp"
#include "Mesh.hpp"
#include "IOUtil.hpp"
#include "WorldObject.hpp"
#include "Voxel.hpp"
#include "ShaderStuff.hpp"

#include "NewChunk.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <array>
#include <print>

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "PerlinNoise.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// stbi_image_write fails to compile without this 
#define sprintf sprintf_s
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// DISCLAIMER: Sometimes I ask shitgpt to write a snippet of code but basically 
//			   everything is my work alone 

// NOTE: I'm basically going to be writing a fundamentally different voxel framework
//       directly over the existing one, with a different output mesh format and a
//		 different grid. Not before I experiment with multi-pass rendering though!

// TODO: Need proper world system before any kind of player-world interactions 

// TODO: Worldgen visualizer suite. Will prolly implement a noise parser
//       and editor somehow. This would allow reconfiguration and testing
//       of worldgen scripts without having to recompile and restart the
//       program.
// TODO: Screen animation framework for easier screen playing 
//		 Loading screens and other such doodads should be subclasses
//       of the WorldObject so that their initialization logic can be
//       contained within their context instead of stitching together
//       parts of the feature in driver code.
// TODO: Shader uniform caching to avoid repeated lookups, possibly provide
//       a uniform setter if possible. (The uniform caching is to avoid
//       requiring uniform address references outside of the shader ctx).
// TODO: Possibly use coroutines and multithreaded execution, run
//		 renderer separate from game loading code. Events and such.
// TODO: Texture loading and management framework 
// TODO: Generalization of above, asset loading and management framework
// TODO: Voxel world loading framework. Finally making something that isnt
//       the fixed demo worldspace.

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

// VIIIIBE COOOODEEEE
void saveFramebufferToPNG(const char* filename, int width, int height) {
	// 1. Set pixel alignment to avoid row shifting/padding issues
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// 2. Allocate buffer for RGB or RGBA data
	std::vector<unsigned char> pixels(width * height * 3);

	// 3. Read pixels from the currently bound framebuffer
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	// 4. OpenGL origin is bottom-left, but PNG origin is top-left. Flip vertically.
	stbi_flip_vertically_on_write(true);

	// 5. Write the PNG file using stb_image_write
	if (!stbi_write_png(filename, width, height, 3, pixels.data(), width * 3)) {
		std::cerr << "Failed to write image: " << filename << std::endl;
	}
}

size_t sum(std::vector<size_t> xs) {
	size_t retval = 0;

	for (auto x : xs) retval += x;

	return retval;
}

int main() {
	std::shared_ptr<mk::DrawContext> ctx = std::shared_ptr<mk::DrawContext>(mk::DrawContext::create_context(600, 400, "Voxel Stuff"));

	mk::ShaderManager shader_manager;

	shader_manager.add_shader("basic", {
		{ GL_VERTEX_SHADER, "basic_vertex_shader.glsl" },
		{ GL_FRAGMENT_SHADER, "basic_fragment_shader.glsl" }
	});

	shader_manager.add_shader("skybox", {
		{ GL_VERTEX_SHADER, "skybox_vertex_shader.glsl" },
		{ GL_FRAGMENT_SHADER, "skybox_fragment_shader.glsl" }
	});

	shader_manager.add_shader("SSBO", {
		{ GL_VERTEX_SHADER, "ssbo_vertex.glsl" },
		{ GL_FRAGMENT_SHADER, "ssbo_fragment.glsl" }
	});

	shader_manager.add_shader("voxel", {
		{ GL_VERTEX_SHADER, "voxel_vertex_shader.glsl" },
		{ GL_FRAGMENT_SHADER, "voxel_fragment_shader.glsl"}
	});

	shader_manager.add_shader("loadingscreen", {
		{ GL_VERTEX_SHADER, "loadingscreen_vertex_shader.glsl" },
		{ GL_FRAGMENT_SHADER, "loadingscreen_fragment_shader.glsl" }
	});

	shader_manager.add_shader("screen", {
		{ GL_VERTEX_SHADER, "screen_vertex.glsl" },
		{ GL_FRAGMENT_SHADER, "screen_fragment.glsl" }
	});

	shader_manager.add_shader("SSAO", {
		{ GL_VERTEX_SHADER, "ssao_vertex.glsl" },
		{ GL_FRAGMENT_SHADER, "ssao_fragment.glsl" }
	});

	shader_manager.add_shader("SSAOblur", {
		{ GL_VERTEX_SHADER, "ssao_blur_vertex.glsl" },
		{ GL_FRAGMENT_SHADER, "ssao_blur_fragment.glsl" }
	});

	std::shared_ptr<mk::Shader> shader = shader_manager.get_shader("basic").value();
	std::shared_ptr<mk::Shader> skybox_shader = shader_manager.get_shader("skybox").value();
	std::shared_ptr<mk::Shader> ssbo_shader = shader_manager.get_shader("SSBO").value();
	std::shared_ptr<mk::Shader> voxel_shader = shader_manager.get_shader("voxel").value();
	std::shared_ptr<mk::Shader> loadingscreen_shader = shader_manager.get_shader("loadingscreen").value();
	std::shared_ptr<mk::Shader> screen_shader = shader_manager.get_shader("screen").value();
	std::shared_ptr<mk::Shader> ssao_shader = shader_manager.get_shader("SSAO").value();
	std::shared_ptr<mk::Shader> ssao_blur_shader = shader_manager.get_shader("SSAOblur").value();

	mk::WorldObject* skybox = new mk::WorldObject;
	skybox->mesh() = std::shared_ptr<mk::Mesh>(mk::Mesh::get_sample_cube()->with_shader(skybox_shader));
	skybox->mesh()->cull_face = GL_FRONT;

	auto mesh = std::shared_ptr<mk::Mesh>(mk::Mesh::get_sample_cube()->with_shader(shader));
	auto cube = new mk::WorldObject;
	cube->mesh() = mesh;

	/*mk::WorldObject* loadingscreen_quad = new mk::WorldObject;
	loadingscreen_quad->mesh() = std::make_shared<mk::Mesh>();
	loadingscreen_quad->mesh()->m_shader_used = loadingscreen_shader;

	loadingscreen_quad->mesh()->vertices = std::vector<mk::Vertex>{
		mk::Vertex{ { -1.f, -1.f,  0.0f,} }, // 0: Bottom-Left
		mk::Vertex{ {-1.f,  1.f,  0.0f,} }, // 1: Top-Left
		mk::Vertex{ { 1.f,  1.f,  0.0f,} }, // 2: Top-Right
		mk::Vertex{ { 1.f, -1.f,  0.0f} },  // 3: Bottom-Right
	};
	loadingscreen_quad->mesh()->indices = std::vector<GLuint>{
		0, 2, 1,  // First Triangle (Bottom-Left, Top-Right, Top-Left)
		0, 3, 2   // Second Triangle (Bottom-Left, Bottom-Right, Top-Right)
	};

	std::println("Creating loadingscreen mesh...");
	loadingscreen_quad->mesh()->init_gl_handle();

	loadingscreen_shader->bind();
	GLuint loadingscreen_u_time = loadingscreen_shader->get_uniform_location("u_time");
	GLuint loadingscreen_u_resolution = loadingscreen_shader->get_uniform_location("u_resolution");

	// NEW YEARS' RESOLUTIONS
	// 1. RUIN EVERYTHING 
	// 2. ATONE 
	// 3. ACCEPT WHAT HAS BEEN LOST 
	// 4. PROTECT WHAT IS YET

	glUniform1f(loadingscreen_u_time, 0.0f);
	glm::ivec2 resolution = {};
	glm::vec2 resolution_f = {};
	glfwGetWindowSize(ctx->window, &resolution.x, &resolution.y);
	resolution_f = { resolution.x, resolution.y };
	glUniform2f(loadingscreen_u_resolution, resolution_f.x, resolution_f.y);

	std::println("loadingscreen mesh done.");

	// loadingscreen texture 
	stbi_set_flip_vertically_on_load(true);
	int ls_width, ls_height, ls_nr_channels;
	// PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE STAND BY PLEASE S
	unsigned char* loadingscreen_uv_data = stbi_load("PLEASESTANDBY.png", &ls_width, &ls_height, &ls_nr_channels, 0x000000000000000000000000000000000000000000);
	if (!loadingscreen_uv_data) {
		std::println("WE CAN NO LONGER STAND BY. THIS UNIVERSE WILL BE DESTROYED.");
		return 1;
	}
	std::println("PLEASESTANDBY loaded");

	GLuint loadingscreen_texture; // shitgpt wrote this section 

	// 1. Generate and bind the texture object
	glGenTextures(1, &loadingscreen_texture);
	glBindTexture(GL_TEXTURE_2D, loadingscreen_texture);

	// 2. Set stretching (wrapping) behaviors to clamp cleanly to full window borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// 3. Set scaling (filtering) methods for stretch optimization
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 4. Determine internal format based on your raw channels
	GLenum format = GL_RGB;
	if (ls_nr_channels == 1)      format = GL_RED;
	else if (ls_nr_channels == 4) format = GL_RGBA;

	// 5. Upload raw data arrays to the GPU
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Prevents corruption if width is not a multiple of 4
	glTexImage2D(GL_TEXTURE_2D, 0, format, ls_width, ls_height, 0, format, GL_UNSIGNED_BYTE, loadingscreen_uv_data);

	unsigned char* loadingscreen_alt_uv_data = stbi_load("STANDBYJUSTALITTLEMORE.png", &ls_width, &ls_height, &ls_nr_channels, 0x00000);

	GLuint loadingscreen_texture_alt;
	// 1. Generate and bind the texture object
	glGenTextures(1, &loadingscreen_texture_alt);
	glBindTexture(GL_TEXTURE_2D, loadingscreen_texture_alt);

	// 2. Set stretching (wrapping) behaviors to clamp cleanly to full window borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// 3. Set scaling (filtering) methods for stretch optimization
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 4. Determine internal format based on your raw channels
	format = GL_RGB;
	if (ls_nr_channels == 1)      format = GL_RED;
	else if (ls_nr_channels == 4) format = GL_RGBA;

	// 5. Upload raw data arrays to the GPU
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Prevents corruption if width is not a multiple of 4
	glTexImage2D(GL_TEXTURE_2D, 0, format, ls_width, ls_height, 0, format, GL_UNSIGNED_BYTE, loadingscreen_alt_uv_data);

	stbi_image_free(loadingscreen_uv_data); // end of the shitgpt section 
	stbi_image_free(loadingscreen_alt_uv_data);

	glfwMaximizeWindow(ctx->window);

	std::println("Drawing loadingscreen?");
	ctx->clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };
	ctx->schedule_mesh_to_draw(loadingscreen_quad);
	ctx->draw();

	// Unbind to prevent accidental modifications
	glBindTexture(GL_TEXTURE_2D, 0);*/ // LOADINGSCREEN

	auto ssbo_obj = new mk::WorldObject;
	ssbo_obj->mesh() = mk::create_ssbo_test_mesh(ssbo_shader->id());
	ssbo_obj->mesh()->with_shader(ssbo_shader);
	ssbo_obj->transform().position = glm::vec3{ -20.0f, 10.0f, 0.0f };
	std::println("SSBO created."); // https://voxel.wiki/wiki/vertex-pulling/
	//ssbo_obj->mesh()->with_shader(ssbo_shader);
	//ssbo_obj->transform().position = glm::vec3{ -20.0f, 10.0f, 0.0f };

	//auto chunk_obj = new mk::WorldObject;
	//auto chunk = std::make_shared<mk::Chunk>();
	using namespace std::chrono;

	const siv::PerlinNoise::seed_type seed = 123456u;
	const siv::PerlinNoise perlin{ seed };

	//for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) for (int x = 0; x < 64; ++x) {
	//	int k = 32;
	//	int h = 32;
	//	int l = 32;

	//	int px = x;
	//	int py = y;
	//	int pz = z;

	//	float radius = 30;

	//	float dp = sqrt((float)(px - k) * (px - k) + (py - h) * (py - h) + (pz - l) * (pz - l));
	//	if (dp < radius) chunk->voxels_set[y][z] |= (1ULL << (63 - x));
	//}

	//chunk_obj->mesh() = std::make_shared<mk::Mesh>(mk::generate_chunk_mesh(*chunk));
	//chunk_obj->mesh()->with_shader(shader);




	constexpr size_t wsz = 8;

	class World {
	public:
		using ChunkGridType = std::array<std::array<mk::Chunk, wsz>, wsz>;

		ChunkGridType* fixedChunks = new ChunkGridType;
	};

	auto world = new std::array<std::array<mk::Chunk, wsz>, wsz>;

	auto gen_start = high_resolution_clock::now();


	std::println("Building world...");

#define max(x, y) (x < y ? y : x)
	for (int cx = 0; cx < wsz; ++cx) for (int cz = 0; cz < wsz; ++cz) {
		world->at(cz).at(cx).voxel_types = new mk::Chunk::VoxelTypeMap();

		for (int z = 0; z < 64; z += 1) for (int y = 0; y < 64; y += 1) {
			world->at(cz).at(cx).voxels_set[y][z] = 0;
		}

		auto chunk = mk::BlockChunk(&world->at(cz).at(cx));

		/*for (int z = 0; z < 16; z += 1) for (int x = 0; x < 16; x += 1) {
			int noise = 1;

			float scale = 0.02;
			noise = perlin.octave2D_01((cx * 16 + x) * scale, (cz * 16 + z) * scale, 4) * 16 + 1;

			for (int y = 0; y <= noise && y < 16; ++y) {
				chunk.set_block_type_at({ x, y, z }, mk::VoxelType::BASE);
			}

			noise = perlin.octave2D_01((cx * 16 + x) * scale*2, (cz * 16 + z) * scale*2, 4) * 16 + 1;

			for (int y = 0; y <= noise && y < 16; ++y) {
				chunk.set_block_type_at({ x, y, z }, mk::VoxelType::BASE);
			}
		}*/

		for (int z = 0; z < 64; z += 1) for (int x = 0; x < 64; x += 1) { // TODO: MAKE VOXEL SCALE PARAMETERS
			for (int y = 0; y < 64; ++y) {
				world->at(cz).at(cx).voxels_set[y][z] &= ~(1ULL << (63 - x));
			}

			float scale = 0.01;
			int noise = perlin.octave2D_01((cx * 64 + x) * scale, (cz * 64 + z) * scale, 4) * 64 + 3;
			noise += 32;
			noise -= perlin.octave2D_01((cx * 64 + x) * (scale / 4), (cz * 64 + z) * (scale / 4), 4) * 64 + 3;
			//noise += 32;
			//noise -= perlin.octave2D_01((cx * 64 + x) * (scale / 8), (cz * 64 + z) * (scale / 8), 4) * 64 + 3;
			if (noise <= 0) noise = 1;
			//if (noise <= 20) noise -= perlin.octave2D_01((cx * 64 + x) * scale, (cz * 64 + z) * scale, 4) * 64 + 3;
			//int noise = (x % 2 == 1 && z % 2 == 0) || (x % 2 == 0 && z % 2 == 1) ? 0 : 8;
			//int noise = 2;

			//int noise = 5;
			//if (x > 16 || z > 16) noise = 3;
			//if (x > 24 || z > 24) noise = 2;
			//if (x > 32 && z > 32) noise = 1;

			mk::VoxelType type_to_set = mk::VoxelType::STONE;
			//if (noise >= 16) type_to_set = mk::VoxelType::TEST;
			//if (x == 32 || z == 32) type_to_set = mk::VoxelType::TEST;

			//if (cx == 0 && cz == 0 && x == 0 && z == 0) {
			//	noise = 1;
			//	//world->at(cz).at(cx).voxels_set[1][z] &= ~(1ULL << 63);
			//}
			//noise = 4;
			//if (x > 48 && z > 48) noise = 2;
			//if (x < 32 && z < 32) noise = 2;

			//if (noise >= 32) type_to_set = mk::VoxelType::TEST;

			//noise = 2;
			//if ((x % 2 == 0 && z % 2 == 0) || (x % 2 == 1 && z % 2 == 1)) type_to_set = mk::VoxelType::TEST;

			//noise = 1;

			for (int y = 0; y < noise && y < 64; ++y) {
				world->at(cz).at(cx).voxels_set[y][z] |= (1ULL << (63 - x));
				world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = type_to_set;

				float value = perlin.octave3D_01((cx * 64 + x) * scale, y * scale, (cz * 64 + z) * scale, 4, 0.5);
				
				//if (y == 63) world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::TEST;
				//if (y < 48) world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::DIRT;
				//if (y < 24) world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::STONE;
				if (y < 1) world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::BEDROCK;

				//chunk.set_block_type_at({ x, y, z }, mk::VoxelType::BASE);
			}

			float carver_scale = 0.01;
			for (int y = 1; y < 64 && (world->at(cz).at(cx).voxels_set[y][z] & (1ULL << (63 - x))) == (1ULL << (63 - x)); ++y) {
				float carver_value = perlin.octave3D_01((cx * 64 + x) * carver_scale, y * carver_scale, (cz * 64 + z) * carver_scale, 3, 0.5);

				if (carver_value < 0.3) {
					world->at(cz).at(cx).voxels_set[y][z] &= ~(1ULL << (63 - x));
					world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::NONE;
				}
			}

			

#define min(x, y) x > y ? y : x
			noise = min(63, noise);
			while (noise > 0 && world->at(cz).at(cx).voxel_types->at(noise - 1).at(z).at(x) == mk::VoxelType::NONE) --noise;

			for (int y = noise - 1; y < noise - 1 + 4 && y < 64; ++y) {
				world->at(cz).at(cx).voxels_set[y][z] |= (1ULL << (63 - x));
				world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::DIRT;
			}
			auto top = noise - 1 + 4 >= 64 ? 63 : noise - 1 + 4;
			world->at(cz).at(cx).voxels_set[top][z] |= (1ULL << (63 - x));
			world->at(cz).at(cx).voxel_types->at(top).at(z).at(x) = mk::VoxelType::GRASS;

			{
				float carver_value = perlin.octave3D_01((cx * 64 + x) * carver_scale, 1 * carver_scale, (cz * 64 + z) * carver_scale, 3, 0.5);
				if (carver_value < 0.3) for (int y = 1; y < 64; ++y) {
					world->at(cz).at(cx).voxels_set[y][z] |= (1ULL << (63 - x));
					world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::STONE;
				}
			}

			
			//if (noise < 63) for (int y = noise - 1; y < 62; ++y) {
			//	if (world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) == mk::VoxelType::DIRT
			//		&& world->at(cz).at(cx).voxel_types->at(y + 1).at(z).at(x) == mk::VoxelType::NONE) {
			//		world->at(cz).at(cx).voxel_types->at(y).at(z).at(x) = mk::VoxelType::GRASS;
			//		world->at(cz).at(cx).voxels_set[y][z] |= (1ULL << (63 - x));
			//	}
			//}

			//if (cx == 0 && cz == 0 && x == 0 && z == 0) world->at(cz).at(cx).voxels_set[3][z] &= ~(1ULL << 63);
			//for (int y = noise*4; y < 64; ++y) world->at(cz).at(cx).voxels_set[y][z] &= ~(1ULL << (63 - x));
		}

		if (cz < wsz - 1) world->at(cz).at(cx).neighbors[mk::FRONT] = &(world->at(cz + 1ULL).at(cx));
		if (cz > 0) world->at(cz).at(cx).neighbors[mk::BACK] = &(world->at(cz - 1ULL).at(cx));
		if (cx > 0) world->at(cz).at(cx).neighbors[mk::LEFT] = &(world->at(cz).at(cx - 1ULL));
		if (cx < wsz - 1) world->at(cz).at(cx).neighbors[mk::RIGHT] = &(world->at(cz).at(cx + 1ULL));

		/*if (cz == 0 && cx == 0) {
			chunk.set_block_type_at({ 0, 0, 0 }, mk::VoxelType::BASE);
			chunk.set_block_type_at({ 1, 0, 0 }, mk::VoxelType::TEST);
			chunk.set_block_type_at({ 0, 0, 1 }, mk::VoxelType::TEST);
			chunk.set_block_type_at({ 1, 0, 1 }, mk::VoxelType::BASE);
		}*/

		//std::println("{}/{}", cx * wsz + cz + 1, wsz * wsz);// LOADINGSCREEN
		//if (cx % 7 == 0) {
		//	ctx->schedule_mesh_to_draw(loadingscreen_quad);
		//	loadingscreen_shader->bind();
		//	glBindTexture(GL_TEXTURE_2D, loadingscreen_texture);
		//	glUniform1f(loadingscreen_u_time, (float)glfwGetTime());
		//	glUniform2f(loadingscreen_u_resolution, resolution_f.x, resolution_f.y);
		//	ctx->draw();
		//}

		if (cz == wsz - 1) {
			int scale = 20;
			int progress = ((float)(cx * wsz + cz + 1)) / (float)(wsz * wsz) * scale;
			std::print("Worldgeneration progress: ");
			for (int i = 0; i <= progress; ++i) {
				std::print("|");
			}
			for (int i = progress; i < scale; ++i) {
				std::print(".");
			}
			std::println();
		}
	}



	// -- New chunk stuff

	std::shared_ptr<mk::pre::World> new_world = std::make_shared<mk::pre::World>();
	new_world->client_settings = std::make_shared<mk::pre::ClientSettings>();
	new_world->client_settings->render_distance = 256;

	new_world->name = "Test";

	//new_world->generator = std::make_shared<mk::pre::WorldGenerator>();



	// -- End of new chunk stuff 



	auto gen_end = high_resolution_clock::now();
	std::println("Generated world in {}ms", duration_cast<milliseconds>(gen_end - gen_start).count());

	std::vector<size_t> times;
	std::vector<size_t> vertex_counts;

	auto world_objs = std::vector<mk::WorldObject*>{};
	/*for (int x = 0; x < wsz; ++x) for (int z = 0; z < wsz; ++z) {
		auto start = high_resolution_clock::now();
		auto obj = new mk::WorldObject;
		world_objs.push_back(obj);

		obj->mesh() = std::make_shared<mk::Mesh>(mk::generate_chunk_mesh(world->at(z).at(x)));
		obj->mesh()->with_shader(shader);
		obj->transform().position = { x * 64 , 0, z * 64 };
		obj->transform().scale = 0.5f;
		vertex_counts.push_back(obj->mesh()->vertices.size());

		auto end = high_resolution_clock::now();

		times.push_back(duration_cast<microseconds>(end - start).count());
	}*/

	//std::println("Avg. meshing time: {}us/64^3 chunk ({}ms total)", sum(times) / times.size(), sum(times)/1000);
	/*size_t sum = 0;
	for (auto s : vertex_counts) sum += s;
	std::println("Avg. vertices per mesh: {}", sum / vertex_counts.size());
	vertex_counts.clear();*/

	auto pstart = high_resolution_clock::now();

	//std::vector<int> xs(16'384);

	//for (int i = 0; i < 16'384; ++i) {
		//xs.push_back(i);
		//xs[i] = i;
	//}

	auto pend = high_resolution_clock::now();

	//chunk_obj->transform().position.x = 50.0f;

	float scale = 0.25f;

	auto start = high_resolution_clock::now();

	//glBindTexture(GL_TEXTURE_2D, loadingscreen_texture_alt);
	//ctx->schedule_mesh_to_draw(loadingscreen_quad);
	//loadingscreen_shader->bind();
	//glUniform1f(loadingscreen_u_time, 0.0f);
	//ctx->draw();// LOADINGSCREEN

	std::println("Generating meshes...");

	size_t vertices = 0;
	std::vector<mk::VoxelChunkObject> voxel_objects;
	for (int x = 0; x < wsz; ++x) for (int z = 0; z < wsz; ++z) {
		auto voxel_object = mk::VoxelChunkObject{};

		voxel_object.mesh = mk::generate_voxel_mesh(world->at(z).at(x));
		voxel_object.shader = voxel_shader;
		voxel_object.transform.position = { x * 64 * scale, 1, z * 64 * scale };
		voxel_object.transform.scale = scale;

		voxel_objects.emplace_back(voxel_object);
		vertices += voxel_object.mesh.vertices.size();
	}

	auto end = high_resolution_clock::now();
	auto duration_us = duration_cast<microseconds>(end - start).count();
	std::println("Meshes generated in {}ms (about {}us/chunk)", duration_us / 1000, duration_us / (wsz * wsz));
	std::println("Total vertices: {} (avg. {}/chunk)", vertices, vertices / (wsz * wsz));

	//int width, height, nr_channels;
	//unsigned char* data = stbi_load("dirt.png", &width, &height, &nr_channels, 0);
	//if (!data) {
	//	std::println(stderr, "oh");
	//}

	//GLuint texture;
	//glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);

	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	////glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	////glGenerateMipmap(GL_TEXTURE_2D);

	//stbi_image_free(data);

	// -- Build array texture 

	int width, height, nr_channels;
	// this should load from textures/voxel_uvs.png but what do i know 
	stbi_set_flip_vertically_on_load(true);
	unsigned char* voxel_uv_data = stbi_load("voxel_uvs.png", &width, &height, &nr_channels, 0);
	if (!voxel_uv_data) {
		std::println("This is not tiki tiki.");
		return 1;
	}

	std::println("Image encoding: {} x {}, {}", width, height, nr_channels);

	std::println("{} {} {} {} {} {}", 
		(unsigned int)voxel_uv_data[0],
		(unsigned int)voxel_uv_data[1],
		(unsigned int)voxel_uv_data[2],
		(unsigned int)voxel_uv_data[3],
		(unsigned int)voxel_uv_data[4],
		(unsigned int)voxel_uv_data[5]);

	// In-place remapping of texture atlas for direct passing to glTexImage3D
	// TODO: Generalize algorithm in case texture atlas dimensions/texture dimensions change
	//       (forwardslash, can change) 

	for (size_t r = 0; r < 256; ++r) {
		size_t col_start = 16 + 16 * (r % 16);

		for (size_t c = col_start; c < 256; ++c) {
			size_t src_px_idx = (r * 256 + c) * nr_channels; // swap "source"

			size_t dst_r_actual = (c / 16) + (r / 16 * 16) - (r / 16); 
			size_t dst_c_offset = (c % 16 + 16 * r);

			size_t dst_px_idx = (dst_r_actual * 256 + dst_c_offset) * nr_channels; // swap "destination"

			unsigned char dst_pxout_lut[3] = {255, 0 + (255 * (r / 16)), 0};

			/*for (unsigned int i = 0; i < nr_channels; ++i) {
				voxel_uv_data[src_px_idx + i] = 255;
				voxel_uv_data[dst_px_idx + i] = dst_pxout_lut[i];
			}*/
			
			for (unsigned int i = 0; i < nr_channels; ++i) {
				std::swap(voxel_uv_data[src_px_idx + i], voxel_uv_data[dst_px_idx + i]);
			}
		}
	}	

	stbi_write_png("test.png", 256, 256, nr_channels, voxel_uv_data, nr_channels * width);
	// end of in-place remapping (DRY? Never heard of her.)

	GLuint array_texture = 0;
	glGenTextures(1, &array_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);

	//glTexStorage3D(GL_TEXTURE_2D_ARRAY,
	//	1,
	//	GL_RGB8,
	//	2, 2,
	//	256);

	//glTexStorage3D(GL_TEXTURE_2D_ARRAY,
	//	1,
	//	GL_RGB8,
	//	2, 2,
	//	256);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, 16, 16, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, voxel_uv_data);
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, 16, 16, 256);

	//// 2. Configure pixel storage row length to match your 256px wide atlas sheet
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, 256); // not wholly sure what the point of this is 

	//// vibecoded snippet doesnt properly emit texture, should figure out on my own
	//// I need to sleep. Figure out how to unpack texture atlas tmw.
	//int layer = 0;
	//for (int gridY = 0; gridY < 16; ++gridY) {
	//	for (int gridX = 0; gridX < 16; ++gridX) {

	//		// Calculate the byte offset to the top-left corner of this specific 16x16 tile
	//		// (Assuming 4 bytes per pixel for GL_RGBA)
	//		int bytesPerPixel = 4;
	//		int offset = ((gridY * 16 * 256) + (gridX * 16)) * bytesPerPixel;

	//		glTexSubImage3D(
	//			GL_TEXTURE_2D_ARRAY,
	//			0,
	//			0, 0, layer,       // Target x, y, and the destination layer index
	//			16, 16, 1,         // Width, Height, Depth (1 layer at a time)
	//			GL_RGBA,
	//			GL_UNSIGNED_BYTE,
	//			(char*)voxel_uv_data + offset // Pointer pushed forward to the tile's starting pixel
	//		);

	//		layer++;
	//	}
	//}

	//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	stbi_image_free(voxel_uv_data);

	//for (GLuint i = 0; i < 256; ++i) {
	//	glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
	//		0,
	//		0, 0, i,
	//		16, 16, 1,
	//		GL_RGB,
	//		GL_UNSIGNED_BYTE,);
	//}
	//for (GLuint i = 0; i < 100; ++i) { // faces towards positive z and negative x are reversed 
	//	int a = rand() % 255;
	//	int b = rand() % 255;
	//	int c = rand() % 255;
	//	int d = rand() % 255;
	//	int e = rand() % 255;
	//	int f = rand() % 255;
	//	GLubyte color[16] = {
	//		a, b, c, d,
	//		e, f, 0, 0,
	//		d, e, f, a,
	//		b, c, 0, 0,
	//	};
	//	glTexSubImage3D(GL_TEXTURE_2D_ARRAY,  // 0x0500 (invalid enum) returned on first call 
	//		0,
	//		0, 0, i,
	//		2, 2, 1,
	//		GL_RGB,
	//		GL_UNSIGNED_BYTE,
	//		color);
	//}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	// -- end of build array texture 

	ctx->far = 10000.0f;
	skybox->mesh()->depth_mask = false;

	int frames = 0;
	start = std::chrono::steady_clock::now();
	int fps_this_time = 0;

	std::chrono::nanoseconds elapsed;

	ctx->clear_color = { 0.0f, 0.5f, 0.5f, 1.0f };

	int i = 0;

	float render_distance = 64.0f;

	ctx->clear_color = mk::Color{ 0.0f };
	ctx->camera().pos.y = 23.0f;
	ctx->camera().pos.x = 50.0f;
	ctx->camera().pos.z = 60.0f;


	// -- framebuffer stuff

	//unsigned int framebuffer;
	//glGenFramebuffers(1, &framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//// generate texture
	//unsigned int textureColorbuffer;
	//glGenTextures(1, &textureColorbuffer);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	auto window_size = ctx->window_size();

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_size.x, window_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//// attach it to currently bound framebuffer object
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	auto SCR_WIDTH = window_size.x;
	auto SCR_HEIGHT = window_size.y;

	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;

	// - position color buffer
	glGenTextures(1, &gPosition);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_size.x, window_size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float quadVertices[] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	// Texture coordinate attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// -- end of framebuffer stuff 

	// -- SSAO stuff 

	// SSAO kernel samples 
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		ssaoKernel.push_back(sample);

		float scale = (float)i / 64.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	// SSAO noise texture 
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// SSAO framebuffer object 
	unsigned int ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// SSAO draw buffer texture 
	unsigned int ssaoColorBuffer;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

	ssao_shader->bind();
	glUniform3fv(ssao_shader->get_uniform_location("samples"), 64, glm::value_ptr(*ssaoKernel.data()));

	// Pipeline: gBuffer draw -> SSAO buffer draw -> SSAO blur -> screen draw 

	// blur 

	unsigned int ssaoBlurFBO, ssaoColorBufferBlur;
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

	// -- end of SSAO stuff 

	ctx->clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	while (not ctx->should_close()) {
		/*if (ctx->regenerate) {
			std::println("{}", ctx->gen_params.scale);
			for (int cx = 0; cx < wsz; ++cx) for (int cz = 0; cz < wsz; ++cz) {
				for (int z = 0; z < 64; ++z) for (int x = 0; x < 64; ++x) {
					int noise = perlin.octave2D_01((cx * 64 + x + ctx->gen_params.offset_x) * ctx->gen_params.scale,
						(cz * 64 + z + ctx->gen_params.offset_z) * ctx->gen_params.scale,
						4) * 64 + 3;

					if (noise >= 64) noise = 63; 

					for (int y = 0; y < noise; ++y) {
						world->at(cz).at(cx).voxels_set[y][z] |= (1ULL << (63 - x));
					}
					for (int y = noise; y < 64; ++y) world->at(cz).at(cx).voxels_set[y][z] &= ~(1ULL << (63 - x));
				}
			}
			ctx->regenerate = false;
		}*/

		/*if (ctx->remesh) {
			std::println("Remeshing world...");
			pstart = high_resolution_clock::now();
			for (auto&& obj : world_objs) {
				int x = (int)(obj->transform().position.x / 64);
				int z = (int)(obj->transform().position.z / 64);
				mk::Mesh tmp = mk::generate_chunk_mesh(world->at(z).at(x), ctx->use_legacy);

				std::swap(tmp.vertices, obj->mesh()->vertices);
				std::swap(tmp.indices, obj->mesh()->indices);
				obj->mesh()->m_gl_mesh_handle.update_mesh(*obj->mesh());
				vertex_counts.push_back(obj->mesh()->vertices.size());
			}

			ctx->remesh = false;
		
			pend = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(pend - pstart).count();
			std::println("Done in {}ms (~{}us per chunk)", duration / 1000, duration / (wsz * wsz));
			sum = 0;
			for (auto s : vertex_counts) sum += s;
			std::println("Avg. vertices per mesh: {}", sum / vertex_counts.size());
			vertex_counts.clear();
		}*/
		
		if (!ctx->toggle_wireframe)
		ctx->schedule_mesh_to_draw(skybox); // lmao? need to look into this.
		//ctx->schedule_mesh_to_draw(load_bearing_coconut); // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
		ctx->schedule_mesh_to_draw(cube); // why even do this if voxel_draw_queue is referred to directly? lazy code.  

		//ctx->schedule_mesh_to_draw(chunk_obj);
		//for (auto&& obj : world_objs) ctx->schedule_mesh_to_draw(obj);

		for (auto&& obj : voxel_objects) {
			auto flat_distance = glm::distance(
				glm::vec3(obj.transform.position.x, 0.0f, obj.transform.position.z),
				glm::vec3(ctx->camera().pos.x, 0.0f, ctx->camera().pos.z));
			//if (flat_distance <= render_distance) {
				ctx->voxel_draw_queue.push(&obj);
			//}
		}

		ctx->schedule_mesh_to_draw(ssbo_obj);

		std::stringstream ss;
		ss << "x: " << (int) ctx->camera().pos.x << " y: " << (int) ctx->camera().pos.y << " z: " << (int) ctx->camera().pos.z << " | " << fps_this_time << "fps";
		std::string title = ss.str();
		glfwSetWindowTitle(ctx->window, title.c_str());

		skybox->transform().pitch = -glm::radians(ctx->camera().pitch);
		skybox->transform().yaw = glm::radians(ctx->camera().yaw);

		//glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);

		// first pass 
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
		glEnable(GL_DEPTH_TEST);

		auto current_window_size = ctx->window_size();
		if (current_window_size != window_size) {
			window_size = current_window_size;
			SCR_WIDTH = window_size.x;
			SCR_HEIGHT = window_size.y;

			// - position color buffer
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

			// - normal color buffer
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

			// - color + specular color buffer
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_size.x, window_size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
			glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			/*glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_size.x, window_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_size.x, window_size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);*/
		}

		ctx->draw();
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		// SSAO pass 
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO); // back to default
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		
		glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssao_shader->bind();
		glUniform1i(ssao_shader->get_uniform_location("gPosition"), 0);
		glUniform1i(ssao_shader->get_uniform_location("gNormal"), 1);
		glUniform1i(ssao_shader->get_uniform_location("texNoise"), 2);

		glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//glUniform3fv(glGetUniformLocation(screen_shader->id(), "viewPos"), 1, glm::value_ptr(ctx->camera().pos));
		//glUniformMatrix4fv()
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)window_size.x / window_size.y, ctx->near, ctx->far);
		glm::mat4 view = glm::identity<glm::mat4>();
		view = glm::lookAt(ctx->camera().pos, ctx->camera().pos + ctx->camera().front, ctx->camera().up);

		glUniformMatrix4fv(ssao_shader->get_uniform_location("projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//glUniformMatrix4fv(ssao_shader->get_uniform_location("view"), 1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// SSAO blur pass 
		ssao_blur_shader->bind();
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// screen draw pass 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

		screen_shader->bind();
		glUniform1i(screen_shader->get_uniform_location("gPosition"), 0);
		glUniform1i(screen_shader->get_uniform_location("gNormal"), 1);
		glUniform1i(screen_shader->get_uniform_location("gAlbedoSpec"), 2);
		glUniform1i(screen_shader->get_uniform_location("gSSAO"), 3);

		glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		

		// draw skybox 
		//ctx->draw_world_object_immediately(skybox);

		glfwSwapBuffers(ctx->window);
		glfwPollEvents();

		++frames;

		end = std::chrono::steady_clock::now();
		elapsed = (end - start);

		if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > 1000) {
			double seconds = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()) / 1000.0;
			fps_this_time = (double)frames / seconds;
			
			frames = 0;
			elapsed = end - end;
			start = end; 
		}
	}
}
