#pragma once 

#include "Mesh.hpp"

#include <algorithm>
#include <iterator>
#include <bitset>
#include <memory>

#include <iostream>

namespace mk {
	constexpr size_t VOXEL_SPACING_CONTIGUOUS = 2.0f; 
	constexpr size_t VOXEL_SPACING = VOXEL_SPACING_CONTIGUOUS;

	struct Voxel {
		bool set = true;
	};

	enum class Neighbor : size_t {
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};

	template <size_t Width, size_t Height=Width, size_t Depth=Width> struct Chunk {
		Voxel voxels[Width][Height][Depth] = {};

		const Chunk* neighbors[6] = {};
	};

	constexpr Vertex dep_[24] = {
		// front 
		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 0 }), // TL 0
		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 1
		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 1, 1 }), // BR 2
		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BL 3

		// back
		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 4
		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 0, 0 }), // TR 5
		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BR 6
		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 1 }), // BL 7

		// left
		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 0, 0 }), // TL 8
		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 9
		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 1, 1 }), // BR 10 
		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BL 11 

		// right 
		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 12
		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 0 }), // TR 13
		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BR 14 
		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 1 }), // BL 15 

		// top 
		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 0 }), // 16
		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // 17
		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 18
		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 1 }), // 19

		// bottom 
		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 20
		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 0 }), // 21
		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 1 }), // 22
		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // 23
	};

	constexpr Vertex cube_base_front[4] = {
		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 0 }), // TL 0
		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 1
		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 1, 1 }), // BR 2
		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BL 3
	};

	//namespace cube_base {
	//	constexpr Vertex front[4] = {
	//		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 0 }), // TL 0
	//		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 1
	//		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 1, 1 }), // BR 2
	//		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BL 3
	//	};
	//	constexpr Vertex back[4] = {
	//		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 4
	//		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 0, 0 }), // TR 5
	//		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BR 6
	//		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 1 }), // BL 7
	//	};
	//	constexpr Vertex left[4] = {
	//		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 0, 0 }), // TL 8
	//		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 9
	//		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 1, 1 }), // BR 10 
	//		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BL 11 
	//	};
	//	constexpr Vertex right[4] = {
	//		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 12
	//		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 0 }), // TR 13
	//		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BR 14 
	//		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 1 }), // BL 15 
	//	};
	//	constexpr Vertex top[4] = {
	//		mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 0 }), // 16
	//		mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // 17
	//		mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 18
	//		mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 1 }), // 19
	//	};
	//	constexpr Vertex bottom[4] = {
	//		mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 20
	//		mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 0 }), // 21
	//		mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 1 }), // 22
	//		mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // 23
	//	};

	namespace cube_base {
		constexpr Vertex front[4] = {
			mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 0, 1 }), // TL 0
			mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 0, 1 }), // TR 1
			mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BR 2
			mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 1 }), // BL 3
		};
		constexpr Vertex back[4] = {
			mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 0, 1 }), // TL 4
			mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 0, 1 }), // TR 5
			mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BR 6
			mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 1 }), // BL 7
		};
		constexpr Vertex left[4] = {
			mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 8
			mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 9
			mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 1, 0 }), // BR 10 
			mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 1, 0 }), // BL 11 
		};
		constexpr Vertex right[4] = {
			mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 0 }), // TL 12
			mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 0 }), // TR 13
			mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 1, 0 }), // BR 14 
			mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 1, 0 }), // BL 15 
		};
		constexpr Vertex top[4] = {
			mk::Vertex({-1,  1,  1}, {0, 0, 0}, { 1, 1 }), // 16
			mk::Vertex({-1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 17
			mk::Vertex({ 1,  1, -1}, {0, 0, 0}, { 1, 1 }), // 18
			mk::Vertex({ 1,  1,  1}, {0, 0, 0}, { 1, 1 }), // 19
		};
		constexpr Vertex bottom[4] = {
			mk::Vertex({-1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 20
			mk::Vertex({-1, -1, -1}, {0, 0, 0}, { 0, 0 }), // 21
			mk::Vertex({ 1, -1, -1}, {0, 0, 0}, { 0, 0 }), // 22
			mk::Vertex({ 1, -1,  1}, {0, 0, 0}, { 0, 0 }), // 23
		};

		constexpr GLuint indices_front[6] = { 3, 1, 0, 3, 2, 1 };
		constexpr GLuint indices_back[6] = { 0, 1, 3, 1, 2, 3 };
		constexpr GLuint indices_left[6] = { 3, 1, 0, 3, 2, 1 };
		constexpr GLuint indices_right[6] = { 0, 1, 3, 1, 2, 3 };
		constexpr GLuint indices_top[6] = { 2, 1, 0, 0, 3, 2 };
		constexpr GLuint indices_bottom[6] = { 0, 1, 3, 1, 2, 3 };
	}

	GLuint dep_2[36] = {
		3, 1, 0, 3, 2, 1,			// front
		4, 5, 7, 5, 6, 7,			// back | normalized: 
		11, 9, 8, 11, 10, 9,		// left 
		12, 13, 15, 13, 14, 15,		// right 
		18, 17, 16, 16, 19, 18,		// top 
		20, 21, 22, 22, 23, 20,		// bottom 
	};

	template <size_t Width, size_t Height = Width, size_t Depth = Width> 
	Mesh get_chunk_mesh(Chunk<Width, Height, Depth>& chunk) {
		Mesh retval;

		// meshing by face instead of by voxel 

		std::vector<glm::ivec3> faces_to_draw_front;
		std::vector<glm::ivec3> faces_to_draw_back;
		std::vector<glm::ivec3> faces_to_draw_left;
		std::vector<glm::ivec3> faces_to_draw_right;
		std::vector<glm::ivec3> faces_to_draw_top;
		std::vector<glm::ivec3> faces_to_draw_bottom;

		// front 
		for (int x = 1; x < Width-1; ++x) for (int y = 1; y < Height-1; ++y) for (int z = 1; z < Depth-1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::FRONT)];

			bool draw_face = false;

			//if (z < Depth - 1) {
				if (!(chunk.voxels[x][y][z + 1].set)) draw_face = true;
			//}
			//else if (!c_face) {
				//draw_face = true;
			//}
			//else if (!c_face->voxels[x][y][0].set) draw_face = true;

			if (draw_face) faces_to_draw_front.push_back({ x, y, z });
		}

		// back
		for (int x = 1; x < Width - 1; ++x) for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth- 1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::BACK)];

			bool draw_face = false;

			//if (z > 0) {
				if (!(chunk.voxels[x][y][z - 1].set)) draw_face = true;
			//}
			//else if (!c_face) {
				//draw_face = true;
			//}
			//else if (!c_face->voxels[x][y][Depth - 1].set) draw_face = true;

			if (draw_face) faces_to_draw_back.push_back({ x, y, z });
		}

		// left 
		for (int x = 1; x < Width-1; ++x) for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth - 1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::LEFT)];

			bool draw_face = false;

			//if (x > 0) {
				if (!(chunk.voxels[x - 1][y][z].set)) draw_face = true;
			//}
			//else if (!c_face) {
				//draw_face = true;
			//}
			//else if (!c_face->voxels[Width-1][y][z].set) draw_face = true;

			if (draw_face) faces_to_draw_left.push_back({ x, y, z });
		}

		// right 
		for (int x = 1; x < Width-1; ++x) for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth - 1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::RIGHT)];

			bool draw_face = false;

			//if (x < Width-1-1) {
				if (!(chunk.voxels[x + 1][y][z].set)) draw_face = true;
			//}
			//else if (!c_face) {
				//draw_face = true;
			//}
			//else if (!c_face->voxels[0][y][z].set) draw_face = true;

			if (draw_face) faces_to_draw_right.push_back({ x, y, z });
		}

		// top 
		for (int x = 1; x < Width-1; ++x) for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth - 1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::UP)];

			bool draw_face = false;

			//if (y < Height - 1 - 1) {
				if (!(chunk.voxels[x][y + 1][z].set)) draw_face = true;
			//}
			//else if (!c_face) {
				//draw_face = true;
			//}
			//else if (!c_face->voxels[x][0][z].set) draw_face = true;

			if (draw_face) faces_to_draw_top.push_back({ x, y, z });
		}

		// bottom 
		for (int x = 1; x < Width-1; ++x) for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth - 1; ++z) {
			if (not chunk.voxels[x][y][z].set) continue;

			size_t idx_offset = retval.vertices.size();

			bool draw_face = false;
			//const Chunk<Width, Height, Depth>* c_face = chunk.neighbors[static_cast<size_t>(Neighbor::DOWN)];

			//bool draw_face = false;

			//if (y > 0) {
				if (!(chunk.voxels[x][y - 1][z].set)) draw_face = true;
			//}
			//else if (!c_face) {
			//	draw_face = true;
			//}
			//else if (!c_face->voxels[x][Height-1][z].set) draw_face = true;

			if (draw_face) faces_to_draw_bottom.push_back({ x, y, z });
		}

		using ThisChunk = Chunk<Width, Height, Depth>;

		// Skip corner voxels, will be read separately to prevent vertex overlap 

		// Reads voxels, NOT FACES. 
		for (int x = 1; x < Width - 1; ++x) for (int y = 1; y < Height - 1; ++y) {
			int z_0 = 0;
			int z_1 = Depth - 1;

			
		}

		for (int x = 1; x < Width - 1; ++x) for (int z = 1; z < Depth - 1; ++z) {
			int y_0 = 0;
			int y_1 = Height - 1;

			
		}

		for (int y = 1; y < Height - 1; ++y) for (int z = 1; z < Depth - 1; ++z) {
			int x_0 = 0;
			int x_1 = Width - 1;

			
		}



		// Getting brain fog on how to do this lol
		/*for (int y = 0; y < Height; ++y) {
			for (int x = 0; x < Width; ++x) {
				int z_0 = 0;
				int z_1 = Depth - 1;

				
			}

			for (int z = 0; z < Depth; ++z) {
				int x_0 = 0;
				int x_1 = Width - 1;

			}
		}*/

		size_t faces_total = faces_to_draw_front.size()
			+ faces_to_draw_back.size()
			+ faces_to_draw_left.size()
			+ faces_to_draw_right.size()
			+ faces_to_draw_top.size()
			+ faces_to_draw_bottom.size();

		retval.vertices.reserve(faces_total * 4);
		retval.indices.reserve(faces_total * 6);

		for (glm::ivec3 offset : faces_to_draw_front) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_front, cube_base::indices_front + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}

		for (glm::ivec3 offset : faces_to_draw_back) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 1 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_back, cube_base::indices_back + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}

		for (glm::ivec3 offset : faces_to_draw_left) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_left, cube_base::indices_left + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
			idx_offset = retval.vertices.size();
		}

		for (glm::ivec3 offset : faces_to_draw_right) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			retval.vertices.push_back(mk::Vertex({  1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 0 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_right, cube_base::indices_right + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
			idx_offset = retval.vertices.size();
		}

		for (glm::ivec3 offset : faces_to_draw_top) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, 1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 1 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, 1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 1 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 1 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 1, 1 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_top, cube_base::indices_top + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
			idx_offset = retval.vertices.size();
		}

		for (glm::ivec3 offset : faces_to_draw_bottom) {
			int x = offset.x, y = offset.y, z = offset.z;
			size_t idx_offset = retval.vertices.size();

			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 0 }));
			retval.vertices.push_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 0 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 0 }));
			retval.vertices.push_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, { 0, 0, 0 }, { 0, 0 }));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_bottom, cube_base::indices_bottom + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
			idx_offset = retval.vertices.size();
		}

		/*for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Height; ++y) {
				for (int z = 0; z < Depth; ++z) {
					if (not chunk.voxels[x][y][z].set) continue;
					// copy indices, add by offset
					size_t idx_offset = retval.vertices.size();
					//std::for_each(indices.begin(), indices.end(), [idx_offset](GLuint& x) { x += idx_offset; });

					Vertex front[4] = {};
					Vertex back[4] = {};
					Vertex left[4] = {};
					Vertex right[4] = {};
					Vertex top[4] = {};
					Vertex bottom[4] = {};

					std::copy(cube_base::front, cube_base::front + 4, front);
					std::copy(cube_base::back, cube_base::back + 4, back);
					std::copy(cube_base::left, cube_base::left + 4, left);
					std::copy(cube_base::right, cube_base::right + 4, right);
					std::copy(cube_base::top, cube_base::top + 4, top);
					std::copy(cube_base::bottom, cube_base::bottom + 4, bottom);

					for (Vertex& v : front) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);
					for (Vertex& v : back) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);
					for (Vertex& v : left) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);
					for (Vertex& v : right) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);
					for (Vertex& v : top) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);
					for (Vertex& v : bottom) v.pos += glm::vec3(VOXEL_SPACING * (float)x, VOXEL_SPACING * (float)y, VOXEL_SPACING * (float)z);

					// -- add exposed faces

					bool draw_front = false;
					bool draw_back = false;
					bool draw_left = false;
					bool draw_right = false;
					bool draw_top = false;
					bool draw_bottom = false;

					using ChunkType = mk::Chunk<Width, Height, Depth>;
					const ChunkType* c_front = chunk.neighbors[static_cast<size_t>(Neighbor::FRONT)];
					const ChunkType* c_back = chunk.neighbors[static_cast<size_t>(Neighbor::BACK)];
					const ChunkType* c_left = chunk.neighbors[static_cast<size_t>(Neighbor::LEFT)];
					const ChunkType* c_right = chunk.neighbors[static_cast<size_t>(Neighbor::RIGHT)];
					const ChunkType* c_up = chunk.neighbors[static_cast<size_t>(Neighbor::UP)];
					const ChunkType* c_down = chunk.neighbors[static_cast<size_t>(Neighbor::DOWN)];

					if (z > 0) {
						if (!chunk.voxels[x][y][z - 1].set) draw_back = true;
					}
					else if (!c_back || !c_back->voxels[x][y][Depth-1].set) {
						draw_back = true; 
					}

					if (z < Depth - 1) {
						if (!(chunk.voxels[x][y][z + 1].set)) draw_front = true;
					}
					else if (!c_front) {
						draw_front = true; 
					}
					else if (!c_front->voxels[x][y][0].set) draw_front = true;

					if (y > 0) {
						if (!chunk.voxels[x][y - 1][z].set) draw_bottom = true;
					}
					else if (!c_down || !c_down->voxels[x][Height-1][z].set) {
						draw_bottom = true; 
					}

					if (y < Height - 1) {
						if (!chunk.voxels[x][y + 1][z].set) draw_top = true;
					}
					else if (!c_up || !c_up->voxels[x][0][z].set) {
						draw_top = true; 
					}

					if (x > 0) {
						if (!chunk.voxels[x - 1][y][z].set) draw_left = true;
					}
					else if (!c_left || !c_left->voxels[Width-1][y][z].set) {
						draw_left = true;
					}

					if (x < Width - 1) {
						if (!chunk.voxels[x + 1][y][z].set) draw_right = true;
					}
					else if (!c_right || !c_right->voxels[0][y][z].set) {
						draw_right = true;
					}

					// - front 
					if (draw_front) {
						std::copy(front, front + 4, std::back_inserter(retval.vertices));
						
						GLuint indices[6] = {};
						std::copy(cube_base::indices_front, cube_base::indices_front + 6, indices);
						for (auto& x : indices) x += idx_offset;
						
						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}
				
					// - back
					if (draw_back) {
						std::copy(back, back + 4, std::back_inserter(retval.vertices));

						GLuint indices[6] = {};
						std::copy(cube_base::indices_back, cube_base::indices_back + 6, indices);
						for (auto& x : indices) x += idx_offset;

						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}

					// - left 
					if (draw_left) {
						std::copy(left, left + 4, std::back_inserter(retval.vertices));

						GLuint indices[6] = {};
						std::copy(cube_base::indices_left, cube_base::indices_left + 6, indices);
						for (auto& x : indices) x += idx_offset;

						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}

					// - right 
					if (draw_right) {
						std::copy(right, right + 4, std::back_inserter(retval.vertices));

						GLuint indices[6] = {};
						std::copy(cube_base::indices_right, cube_base::indices_right + 6, indices);
						for (auto& x : indices) x += idx_offset;

						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}

					// - top 
					if (draw_top) {
						std::copy(top, top + 4, std::back_inserter(retval.vertices));

						GLuint indices[6] = {};
						std::copy(cube_base::indices_top, cube_base::indices_top + 6, indices);
						for (auto& x : indices) x += idx_offset;

						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}

					// - bottom 
					if (draw_bottom) {
						std::copy(bottom, bottom + 4, std::back_inserter(retval.vertices));

						GLuint indices[6] = {};
						std::copy(cube_base::indices_bottom, cube_base::indices_bottom + 6, indices);
						for (auto& x : indices) x += idx_offset;

						std::copy(indices, indices + 6, std::back_inserter(retval.indices));

						idx_offset = retval.vertices.size();
					}
				}
			}
		}*/

		return retval;
	}
}

// -- old meshing code vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
/*mk::Mesh greedy_meshing(const mk::Chunk& chunk) {
	using namespace mk;
	Mesh retval;

	//retval.vertices.reserve(10000);
	//retval.indices.reserve(20000);

	//auto front_mask = get_chunk_front_mask(chunk);
	//auto back_mask = get_chunk_back_mask(chunk);
	//auto left_mask = get_chunk_left_mask(chunk);
	//auto right_mask = get_chunk_right_mask(chunk);
	auto top_mask = get_chunk_top_mask(chunk);
	auto bottom_mask = get_chunk_bottom_mask(chunk);
	auto front_mask = get_front_faces(chunk);
	auto right_mask = get_right_faces(chunk);
	auto left_mask = get_left_faces(chunk);
	auto back_mask = get_back_faces(chunk);

	std::vector<OffsetList> offsets_to_draw_top;
	std::vector<OffsetList> offsets_to_draw_bottom;
	std::vector<OffsetList> offsets_to_draw_front;
	std::vector<OffsetList> offsets_to_draw_right;
	std::vector<OffsetList> offsets_to_draw_left;
	std::vector<OffsetList> offsets_to_draw_back;

	size_t reserve = 0;

	for (int y = 0; y < 64; ++y) {
		auto top_faces = get_faces(top_mask->voxels_set.at(y));
		auto bottom_faces = get_faces(bottom_mask->voxels_set.at(y));
		auto front_faces = get_faces(front_mask->voxels_set.at(y));
		auto right_faces = get_faces(right_mask->voxels_set.at(y));
		auto left_faces = get_faces(left_mask->voxels_set.at(y));
		auto back_faces = get_faces(back_mask->voxels_set.at(y));

		reserve += top_faces.size() + front_faces.size() + right_faces.size() + bottom_faces.size() + left_faces.size() + back_faces.size();

		offsets_to_draw_top.push_back(top_faces);
		offsets_to_draw_front.push_back(front_faces);
		offsets_to_draw_right.push_back(right_faces);
		offsets_to_draw_bottom.push_back(bottom_faces);
		offsets_to_draw_left.push_back(left_faces);
		offsets_to_draw_back.push_back(back_faces);
	}

	retval.vertices.reserve(reserve * 4);
	retval.indices.reserve(reserve * 6);

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_top.at(y);

		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1-1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1-1;

			// (0, 0) ... (x, 0)
			// .               .
			// .               .
			// .               .
			// (0, z) ... (x, z)

			mk::Vertex verts[4] = { mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, 1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z1 }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, 1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z0 }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z0 }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z1 }, 1) };
			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_top, cube_base::indices_top + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_front.at(y);


		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			// (0, 0) ... (x, 0)
			// .               .
			// .               .
			// .               .
			// (0, y) ... (x, y)

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1 - 1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1 - 1;

			mk::Vertex verts[4] = { mk::Vertex({ -1 + VOXEL_SPACING * (float)x0,  1 + VOXEL_SPACING * (float)z1,  1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  1 + VOXEL_SPACING * (float)z1,  1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1, -1 + VOXEL_SPACING * (float)z0,  1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, -1 + VOXEL_SPACING * (float)z0,  1 + VOXEL_SPACING * (float)y }, 1) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_front, cube_base::indices_front + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_back.at(y);


		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			// (0, 0) ... (x, 0)
			// .               .
			// .               .
			// .               .
			// (0, y) ... (x, y)

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1 - 1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1 - 1;

			mk::Vertex verts[4] = { mk::Vertex({ -1 + VOXEL_SPACING * (float)x0,  1 + VOXEL_SPACING * (float)z1,  -1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  1 + VOXEL_SPACING * (float)z1,  -1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)x1, -1 + VOXEL_SPACING * (float)z0,  -1 + VOXEL_SPACING * (float)y }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, -1 + VOXEL_SPACING * (float)z0,  -1 + VOXEL_SPACING * (float)y }, 1) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_back, cube_base::indices_back + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_right.at(y);

		for (int i = 0; i < list.size(); ++i) {
			//int x = offset.x, y = offset.y, z = offset.z;
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			// (z, y) ... (0, y)
			// .               .
			// .               .
			// .               .
			// (z, 0) ... (0, 0)

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1 - 1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1 - 1;

			mk::Vertex verts[4] = { mk::Vertex({ 1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z1,  -1 + VOXEL_SPACING * (float)x0 }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z1,  1 + VOXEL_SPACING * (float)x1 }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)y, -1 + VOXEL_SPACING * (float)z0,  1 + VOXEL_SPACING * (float)x1 }, 1),
									mk::Vertex({ 1 + VOXEL_SPACING * (float)y, -1 + VOXEL_SPACING * (float)z0,  -1 + VOXEL_SPACING * (float)x0 }, 1) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_right, cube_base::indices_right + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_bottom.at(y);

		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1 - 1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1 - 1;

			retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z1 }, 1));
			retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x0, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z0 }, 1));
			retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z0 }, 1));
			retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x1,  -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z1 }, 1));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_bottom, cube_base::indices_bottom + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_left.at(y);

		for (int i = 0; i < list.size(); ++i) {
			//int x = offset.x, y = offset.y, z = offset.z;
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			// (z, y) ... (0, y)
			// .               .
			// .               .
			// .               .
			// (z, 0) ... (0, 0)

			float x0 = (float)offsets.x0;
			float x1 = (float)offsets.x1 - 1;
			float z0 = (float)offsets.z0;
			float z1 = (float)offsets.z1 - 1;

			mk::Vertex verts[4] = { mk::Vertex({ -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z1,  -1 + VOXEL_SPACING * (float)x0 }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z1,  1 + VOXEL_SPACING * (float)x1 }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)y, -1 + VOXEL_SPACING * (float)z0,  1 + VOXEL_SPACING * (float)x1 }, 1),
									mk::Vertex({ -1 + VOXEL_SPACING * (float)y, -1 + VOXEL_SPACING * (float)z0,  -1 + VOXEL_SPACING * (float)x0 }, 1) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_left, cube_base::indices_left + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	//std::println("Generated {} vertices w/ {} indices", retval.vertices.size(), retval.indices.size());

	return retval;
}*/

/*mk::Mesh mk::generate_chunk_mesh(const Chunk& chunk, bool use_legacy) {
	Mesh retval;

	if (!use_legacy) {
		return greedy_meshing(chunk);
	}
	auto front_mask = get_chunk_front_mask(chunk);
	auto back_mask = get_chunk_back_mask(chunk);
	auto left_mask = get_chunk_left_mask(chunk);
	auto right_mask = get_chunk_right_mask(chunk);
	auto top_mask = get_chunk_top_mask(chunk);
	auto bottom_mask = get_chunk_bottom_mask(chunk);

	std::vector<glm::ivec3> faces_to_draw_front;
	std::vector<glm::ivec3> faces_to_draw_back;
	std::vector<glm::ivec3> faces_to_draw_left;
	std::vector<glm::ivec3> faces_to_draw_right;
	std::vector<glm::ivec3> faces_to_draw_top;
	std::vector<glm::ivec3> faces_to_draw_bottom;

	for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) {
		if (chunk.voxels_set[y][z] == 0x0) continue;

		int min = 0;
		int max = 64;

		for (int x = min; x < max; ++x) {
			if (front_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_front.emplace_back(glm::ivec3{ x, y, z });
			}
			if (back_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_back.emplace_back(glm::ivec3{ x, y, z });
			}
			if (left_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_left.emplace_back(glm::ivec3{ x, y, z });
			}
			if (right_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_right.emplace_back(glm::ivec3{ x, y, z });
			}
			if (use_legacy && top_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_top.emplace_back(glm::ivec3{ x, y, z });
			}
			if (bottom_mask->voxels_set[y][z] & (1ULL << (63 - x))) {
				faces_to_draw_bottom.emplace_back(glm::ivec3{ x, y, z });
			}
		}
	}

	if (!use_legacy)  faces_to_draw_top.clear();

	size_t faces_total = faces_to_draw_front.size()
		+ faces_to_draw_back.size()
		+ faces_to_draw_left.size()
		+ faces_to_draw_right.size()
		+ faces_to_draw_top.size()
		+ faces_to_draw_bottom.size();

	retval.vertices.reserve(faces_total * 4 + retval.vertices.size());
	retval.indices.reserve(faces_total * 6 + retval.indices.size());

	for (glm::ivec3 offset : faces_to_draw_front) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 1));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 1));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 1));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 1));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_front, cube_base::indices_front + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
	}

	for (glm::ivec3 offset : faces_to_draw_back) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 2));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 2));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 2));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 2));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_back, cube_base::indices_back + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
	}

	for (glm::ivec3 offset : faces_to_draw_left) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 3));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 3));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 3));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 3));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_left, cube_base::indices_left + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		//idx_offset = retval.vertices.size();
	}

	for (glm::ivec3 offset : faces_to_draw_right) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 4));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 4));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  1 + VOXEL_SPACING * (float)z }, 4));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 4));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_right, cube_base::indices_right + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		//idx_offset = retval.vertices.size();
	}

	if (use_legacy) for (glm::ivec3 offset : faces_to_draw_top) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, 1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, 5));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, 1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 5));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 5));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, 5));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_top, cube_base::indices_top + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		//idx_offset = retval.vertices.size();
	}

	for (glm::ivec3 offset : faces_to_draw_bottom) {
		int x = offset.x, y = offset.y, z = offset.z;
		size_t idx_offset = retval.vertices.size();

		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, 6));
		retval.vertices.emplace_back(mk::Vertex({ -1 + VOXEL_SPACING * (float)x, -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 6));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  -1 + VOXEL_SPACING * (float)y,  -1 + VOXEL_SPACING * (float)z }, 6));
		retval.vertices.emplace_back(mk::Vertex({ 1 + VOXEL_SPACING * (float)x,  -1 + VOXEL_SPACING * (float)y,   1 + VOXEL_SPACING * (float)z }, 6));
		GLuint indices[6] = {};
		std::copy(cube_base::indices_bottom, cube_base::indices_bottom + 6, indices);
		for (auto& x : indices) x += idx_offset;
		std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		//idx_offset = retval.vertices.size();
	}

	return retval;
}*/




/*
struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };

struct Quadface { Vertex top_left; Vertex top_right; Vertex bottom_left; Vertex bottom_right; };

struct Chunk3D {
	std::array<std::array<uint64_t, 64>, 64> voxels_set;
};

// TODO: pre-generate vertex LUT to avoid redundant calculations

std::unique_ptr<Chunk3D> get_chunk_front_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 63; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z + 1ULL));
		}
		retval->voxels_set.at(y).at(63) = chunk.voxels_set.at(y).at(63);
	}

	return retval;
}

std::unique_ptr<Chunk3D> get_chunk_back_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 1; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = ~chunk.voxels_set.at(y).at(z) & (chunk.voxels_set.at(y).at(z - 1ULL));
		}
		retval->voxels_set.at(y).at(0) = chunk.voxels_set.at(y).at(0);
	}

	return retval;
}

std::unique_ptr<Chunk3D> get_chunk_left_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z) >> 1);
		}
	}

	return retval;
}

std::unique_ptr<Chunk3D> get_chunk_right_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z) << 1);
		}
	}

	return retval;
}

std::unique_ptr<Chunk3D> get_chunk_top_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 0; y < 63; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~chunk.voxels_set.at(y + 1ULL).at(z);
		}
	}
	for (int z = 0; z < 64; ++z) {
		retval->voxels_set.at(63).at(z) = chunk.voxels_set.at(63).at(z);
	}

	return retval;
}

std::unique_ptr<Chunk3D> get_chunk_bottom_mask(const Chunk3D& chunk) {
	auto retval = std::make_unique<Chunk3D>();

	for (int y = 1; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~chunk.voxels_set.at(y - 1ULL).at(z);
		}
	}
	for (int z = 0; z < 64; ++z) {
		retval->voxels_set.at(0).at(z) = chunk.voxels_set.at(0).at(z);
	}

	return retval;
}
*/