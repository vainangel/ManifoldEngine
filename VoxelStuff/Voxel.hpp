#pragma once 

#include <cstdint>
#include <memory>
#include <array>

#include "Mesh.hpp"

namespace mk {
	constexpr size_t FRONT = 0;
	constexpr size_t BACK = 1;
	constexpr size_t LEFT = 2;
	constexpr size_t RIGHT = 3;
	constexpr size_t TOP = 4;
	constexpr size_t BOTTOM = 5;

	enum class VoxelType : uint8_t {
		NONE = 4,
		BASE = 254,
		TEST = 253,
		DIRT = 1,
		GRASS = 3,
		STONE = 0,
		BEDROCK = 15,
	};

	/// <summary>
	/// A 64^3 cubic chunk. 
	/// </summary>
	struct Chunk {
		using VoxelTypeMap = std::array<std::array<std::array<VoxelType, 64>, 64>, 64>;

		std::array<std::array<uint64_t, 64>, 64> voxels_set;
		VoxelTypeMap* voxel_types = nullptr; // replace with smart ptr

		Chunk* neighbors[6];
	};

	/// <summary>
	/// Chunk wrapper object, used to set and read blocks at a given location. 
	/// </summary>
	struct BlockChunk {
		Chunk* chunk;

		/// <summary>
		/// Sets the block at `pos` to the corresponding `type`, and to nothing if `type` is `VoxelType::NONE`.
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="type"></param>
		void set_block_type_at(glm::tvec3<uint8_t> pos, VoxelType type);

		VoxelType get_block_type_at(glm::tvec3<uint8_t> pos);
	};

	/// <summary>
	/// Generate a mesh corresponding to this chunk. 
	/// </summary>
	/// <param name="chunk"></param>
	/// <returns></returns>
	mk::Mesh generate_chunk_mesh(const Chunk& chunk, bool use_legacy = false /* tf is "legacy"? this project is NEW */);

	mk::VoxelMesh generate_voxel_mesh(const Chunk& chunk, bool use_legacy = false);

	
	// I think this is unused now 
	constexpr size_t VOXEL_SPACING_CONTIGUOUS = 2.0f;
	constexpr size_t VOXEL_SPACING = VOXEL_SPACING_CONTIGUOUS;

	namespace cube_base {
		//constexpr Vertex front[4] = {
		//	mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 1 }), // TL 0
		//	mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 1 }), // TR 1
		//	mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BR 2
		//	mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BL 3
		//};
		//constexpr Vertex back[4] = {
		//	mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 0, 1 }), // TL 4
		//	mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 0, 1 }), // TR 5
		//	mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BR 6
		//	mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BL 7
		//};
		//constexpr Vertex left[4] = {
		//	mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 8
		//	mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 9
		//	mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 1, 0 }), // BR 10 
		//	mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 0 }), // BL 11 
		//};
		//constexpr Vertex right[4] = {
		//	mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 12
		//	mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 13
		//	mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 1, 0 }), // BR 14 
		//	mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 0 }), // BL 15 
		//};
		//constexpr Vertex top[4] = {
		//	mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 1 }), // 16
		//	mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 17
		//	mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 18
		//	mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 1 }), // 19
		//};
		//constexpr Vertex bottom[4] = {
		//	mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 20
		//	mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 0 }), // 21
		//	mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 0 }), // 22
		//	mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 23
		//};

		// TODO: Remap reference cube for consistent face orientation,
		// TODO: Map reference cube directly to voxel faces (possibly) 
		constexpr GLuint indices_front[6] = { 3, 1, 0, 3, 2, 1 };
		//constexpr GLuint indices_front[6] = { 0, 1, 3, 1, 2, 3 }; // adjusted for consistent texture mapping 
		constexpr GLuint indices_back[6] = { 0, 1, 3, 1, 2, 3 };
		constexpr GLuint indices_left[6] = { 3, 1, 0, 3, 2, 1 };
		constexpr GLuint indices_right[6] = { 0, 1, 3, 1, 2, 3 };
		constexpr GLuint indices_top[6] = { 2, 1, 0, 0, 3, 2 };
		constexpr GLuint indices_bottom[6] = { 0, 1, 3, 1, 2, 3 };
	}
}
