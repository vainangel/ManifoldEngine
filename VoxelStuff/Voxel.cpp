#include "Voxel.hpp"

#include <iterator>
#include <memory>
#include <print>

static std::unique_ptr<mk::Chunk> get_chunk_front_mask(const mk::Chunk& chunk);
static std::unique_ptr<mk::Chunk> get_chunk_back_mask(const mk::Chunk& chunk);
static std::unique_ptr<mk::Chunk> get_chunk_left_mask(const mk::Chunk& chunk);
static std::unique_ptr<mk::Chunk> get_chunk_right_mask(const mk::Chunk& chunk);
static std::unique_ptr<mk::Chunk> get_chunk_top_mask(const mk::Chunk& chunk);
static std::unique_ptr<mk::Chunk> get_chunk_bottom_mask(const mk::Chunk& chunk);

struct QuadfaceOffsets {
	uint8_t x0;
	uint8_t x1;
	uint8_t z0;
	uint8_t z1;
	uint8_t type_id;
};

using OffsetList = std::vector<QuadfaceOffsets>;

enum class ChunkMapping {
	TOP_BOTTOM,
	FRONT_BACK,
	LEFT_RIGHT,
	FRONT,
	BACK,
	LEFT,
	RIGHT,
};

/// <summary>
/// Retrieve the type of a voxel at a given location. The (x, y, z) coordinate is internally remapped as necessary by way of `mapping`.
/// </summary>
/// <param name="chunk"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <param name="mapping"></param>
/// <returns></returns>
static mk::VoxelType get_voxel_type_at(const mk::Chunk& chunk, uint8_t x, uint8_t y, uint8_t z, ChunkMapping mapping) {
	/*if (mapping == ChunkMapping::FRONT) return chunk.voxel_types->at(z).at(y).at(x);
	if (mapping == ChunkMapping::BACK) return chunk.voxel_types->at(z).at(y).at(x);*/

	if (mapping == ChunkMapping::FRONT_BACK) return chunk.voxel_types->at(z).at(y).at(x);
	if (mapping == ChunkMapping::LEFT_RIGHT) return chunk.voxel_types->at(z).at(x).at(y);

	return chunk.voxel_types->at(y).at(z).at(x);
}

// TODO
// Note that chunk rotation (for sequentiality of data access, encouraging vectorized operations) might not provide
// enough of a performance benefit for the time and space it takes to allocate the necessary data is worth. Consider 
// implementing a non-linear-mapping version of the greedy mesher and compare runtimes. If it is considerably faster 
// via "naive" accesses then there is no reason to complicate the greedy meshing algorithm. 

static OffsetList get_faces(std::array<uint64_t, 64> plane, const mk::Chunk& chunk, uint8_t segment=0, ChunkMapping mapping=ChunkMapping::TOP_BOTTOM) { // end-exclusive 
	OffsetList retval = {};

	uint8_t current_row = 0;

	while (current_row < 64) {
		auto& row = plane[current_row];
		if (!row) { ++current_row; continue; }

		uint8_t start = 0;
		uint8_t end = 0;

		// skip unset
		while (!(row & (1ULL << (63 - start))) && start < 64) { 
			// texture check unnecessary here, skipping unset 
			++start;
		}

		mk::VoxelType type = get_voxel_type_at(chunk, start, segment, current_row, mapping);
		uint8_t texture = static_cast<uint8_t>(type);

		end = start;
		while ((row & (1ULL << (63 - end))) && end < 64) { 
			// do texture check here 
			if (type != get_voxel_type_at(chunk, end, segment, current_row, mapping)) break; 
			++end; 
		}

		// if start = 0, end = 8, should generate 0b111111110000000000000000....00000000
		uint64_t mask = row;
		int offset = 64 - end;
		mask >>= offset;
		mask <<= offset;

		uint8_t z_offset = 0;
		//plane[current_row] &= ~mask;
		while (z_offset < 64 - current_row) {
			auto& row_offset = plane[current_row + z_offset];
			if ((row_offset & mask) != mask) break;

			for (uint8_t i = start; i < end; ++i) {
				if (type != get_voxel_type_at(chunk, i, segment, current_row + z_offset, mapping)) goto type_mismatch; 
			}

			row_offset &= ~mask; 
			++z_offset;
		}
		type_mismatch:


		retval.emplace_back(QuadfaceOffsets{ start, end, current_row, (uint8_t)(current_row + z_offset), texture });
	}

	return retval;
}

// TODO: These face masking functions utilize the Chunk type as a temporary voxel face container. It might be idiomatic 
//       to replace this with a dedicated chunk face mask class definition to make the intent of this data clearer, though
//       it wouldn't change anything else. 
static std::unique_ptr<mk::Chunk> get_front_faces(const mk::Chunk& chunk) {
	using namespace mk;

	// TODO: Generate a "rotated" bitmask s.t. front faces are indexed in terms of (x, y) on planes along the z axis.
	
	/* y  0   1     z  0   1
		|aa  cc| --> |cc  dd|
		|bb, dd|     |aa, bb|
	*/ // on the bright side, i dont think i need to change any bitsets... 
	
	auto front_mask = get_chunk_front_mask(chunk);
	auto front_mask_rotated = std::make_unique<Chunk>(); // TODO: Replace with in-place swapping 
	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			front_mask_rotated->voxels_set.at(z).at(y) = front_mask->voxels_set.at(y).at(z);
		}
	}

	return front_mask_rotated;
}

static std::unique_ptr<mk::Chunk> get_back_faces(const mk::Chunk& chunk) {
	using namespace mk;

	// TODO: Generate a "rotated" bitmask s.t. front faces are indexed in terms of (x, y) on planes along the z axis.

	/* y  0   1     z  0   1
		|aa  cc| --> |cc  dd|
		|bb, dd|     |aa, bb|
	*/ // on the bright side, i dont think i need to change any bitsets... 

	auto front_mask = get_chunk_back_mask(chunk);
	auto front_mask_rotated = std::make_unique<Chunk>(); // TODO: Replace with in-place swapping 
	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			front_mask_rotated->voxels_set.at(z).at(y) = front_mask->voxels_set.at(y).at(z);
		}
	}

	return front_mask_rotated;
}

static std::unique_ptr<mk::Chunk> get_right_faces(const mk::Chunk& chunk) {
	using namespace mk;

	auto right_mask = get_chunk_right_mask(chunk);
	auto right_mask_rotated = std::make_unique<Chunk>();
	auto right_mask_rotated_again = std::make_unique<Chunk>();

	for (int y = 0; y < 64; ++y) {
		std::array<uint64_t, 64> xs = {};
		for (int z = 0; z < 64; ++z) {
			auto n = right_mask->voxels_set.at(y).at(z);
			for (int x = 0; x < 64; ++x) {
				auto bit = (n & (1ULL << (63 - x))) >> (63 - x);
				xs[x] |= bit << (63 - z);
			}
		}

		right_mask_rotated->voxels_set.at(y) = xs;
	}

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			right_mask_rotated_again->voxels_set.at(z).at(y) = right_mask_rotated->voxels_set.at(y).at(z);
		}
	}

	return right_mask_rotated_again; 
}

static std::unique_ptr<mk::Chunk> get_left_faces(const mk::Chunk& chunk) {
	using namespace mk;

	auto right_mask = get_chunk_left_mask(chunk);
	auto right_mask_rotated = std::make_unique<Chunk>();
	auto right_mask_rotated_again = std::make_unique<Chunk>();

	for (int y = 0; y < 64; ++y) {
		std::array<uint64_t, 64> xs = {};
		for (int z = 0; z < 64; ++z) {
			auto n = right_mask->voxels_set.at(y).at(z);
			for (int x = 0; x < 64; ++x) {
				auto bit = (n & (1ULL << (63 - x))) >> (63 - x);
				xs[x] |= bit << (63 - z);
			}
		}

		right_mask_rotated->voxels_set.at(y) = xs;
	}

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			right_mask_rotated_again->voxels_set.at(z).at(y) = right_mask_rotated->voxels_set.at(y).at(z);
		}
	}

	return right_mask_rotated_again;
}

mk::VoxelMesh mk::generate_voxel_mesh(const Chunk& chunk, bool use_legacy) {
	mk::VoxelMesh retval;

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

	for (int y = 0; y < 64; ++y) { // TODO: Make each face better-defined. It's difficult to keep track of things. Need to document.
		auto top_faces = get_faces(top_mask->voxels_set.at(y), chunk, y, ChunkMapping::TOP_BOTTOM);
		auto bottom_faces = get_faces(bottom_mask->voxels_set.at(y), chunk, y, ChunkMapping::TOP_BOTTOM);
		auto front_faces = get_faces(front_mask->voxels_set.at(y), chunk, y, ChunkMapping::FRONT_BACK);
		//auto front_faces = OffsetList{};
		auto back_faces = get_faces(back_mask->voxels_set.at(y), chunk, y, ChunkMapping::FRONT_BACK);
		auto left_faces = get_faces(left_mask->voxels_set.at(y), chunk, y, ChunkMapping::LEFT_RIGHT);
		//auto left_faces = OffsetList{};
		auto right_faces = get_faces(right_mask->voxels_set.at(y), chunk, y, ChunkMapping::LEFT_RIGHT);

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

	constexpr uint8_t n_tx = (1 << 7);
	constexpr uint8_t n_ty = (1 << 7);
	constexpr uint8_t n_tz = (1 << 7);

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_top.at(y);

		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			// (0, 0) ... (x, 0)
			// .               .
			// .               .
			// .               .
			// (0, z) ... (x, z)

			mk::VoxelMeshVertex verts[4] = { 
				mk::VoxelMeshVertex({ n_tx + 0 + x0, n_ty + 1 + y, n_tz + 1 + z1 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_tx + 0 + x0, n_ty + 1 + y, n_tz + 0 + z0 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_tx + 1 + x1, n_ty + 1 + y, n_tz + 0 + z0 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_tx + 1 + x1, n_ty + 1 + y, n_tz + 1 + z1 }, offsets.type_id) };
			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_top, cube_base::indices_top + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	constexpr uint8_t n_fx = (0 << 7);
	constexpr uint8_t n_fy = (0 << 7);
	constexpr uint8_t n_fz = (1 << 7);

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

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			mk::VoxelMeshVertex verts[4] = { 
				mk::VoxelMeshVertex({ n_fx + 0 + x0, n_fy + 1 + z1, n_fz + 1 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_fx + 1 + x1, n_fy + 1 + z1, n_fz + 1 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_fx + 1 + x1, n_fy + 0 + z0, n_fz + 1 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_fx + 0 + x0, n_fy + 0 + z0, n_fz + 1 + y }, offsets.type_id) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_front, cube_base::indices_front + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	constexpr uint8_t n_kx = (0 << 7);
	constexpr uint8_t n_ky = (1 << 7);
	constexpr uint8_t n_kz = (0 << 7);

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

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			mk::VoxelMeshVertex verts[4] = {	
				mk::VoxelMeshVertex({ n_kx + 0 + x0, n_ky + 1 + z1, n_kz + 0 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_kx + 1 + x1, n_ky + 1 + z1, n_kz + 0 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_kx + 1 + x1, n_ky + 0 + z0, n_kz + 0 + y }, offsets.type_id),
				mk::VoxelMeshVertex({ n_kx + 0 + x0, n_ky + 0 + z0, n_kz + 0 + y }, offsets.type_id) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_back, cube_base::indices_back + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	constexpr uint8_t n_rx = (1 << 7);
	constexpr uint8_t n_ry = (0 << 7);
	constexpr uint8_t n_rz = (0 << 7);

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

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			mk::VoxelMeshVertex verts[4] = {	
				mk::VoxelMeshVertex({ n_rx + 1 + y, n_ry + 1 + z1, n_rz + 0 + x0 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_rx + 1 + y, n_ry + 1 + z1, n_rz + 1 + x1 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_rx + 1 + y, n_ry + 0 + z0, n_rz + 1 + x1 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_rx + 1 + y, n_ry + 0 + z0, n_rz + 0 + x0 }, offsets.type_id) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_right, cube_base::indices_right + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	constexpr uint8_t n_bx = (0 << 7); // not even gonna do this one lol
	constexpr uint8_t n_by = (0 << 7);
	constexpr uint8_t n_bz = (0 << 7);

	for (int y = 0; y < 64; ++y) {
		OffsetList& list = offsets_to_draw_bottom.at(y);

		for (int i = 0; i < list.size(); ++i) {
			QuadfaceOffsets offsets = list.at(i);
			size_t idx_offset = retval.vertices.size();

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			retval.vertices.emplace_back(mk::VoxelMeshVertex({ 0 + x0, 0 + y, 1 + z1 }, offsets.type_id));
			retval.vertices.emplace_back(mk::VoxelMeshVertex({ 0 + x0, 0 + y, 0 + z0 }, offsets.type_id));
			retval.vertices.emplace_back(mk::VoxelMeshVertex({ 1 + x1, 0 + y, 0 + z0 }, offsets.type_id));
			retval.vertices.emplace_back(mk::VoxelMeshVertex({ 1 + x1, 0 + y, 1 + z1 }, offsets.type_id));
			GLuint indices[6] = {};
			std::copy(cube_base::indices_bottom, cube_base::indices_bottom + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	constexpr uint8_t n_lx = (1 << 7);
	constexpr uint8_t n_ly = (0 << 7);
	constexpr uint8_t n_lz = (1 << 7);

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

			uint8_t x0 = offsets.x0;
			uint8_t x1 = offsets.x1 - 1;
			uint8_t z0 = offsets.z0;
			uint8_t z1 = offsets.z1 - 1;

			mk::VoxelMeshVertex verts[4] = {	
				mk::VoxelMeshVertex({ n_lx + 0 + y, n_ly + 1 + z1, n_lz + 0 + x0 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_lx + 0 + y, n_ly + 1 + z1, n_lz + 1 + x1 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_lx + 0 + y, n_ly + 0 + z0, n_lz + 1 + x1 }, offsets.type_id),
				mk::VoxelMeshVertex({ n_lx + 0 + y, n_ly + 0 + z0, n_lz + 0 + x0 }, offsets.type_id) };

			std::copy(verts, verts + 4, std::back_inserter(retval.vertices));

			GLuint indices[6] = {};
			std::copy(cube_base::indices_left, cube_base::indices_left + 6, indices);
			for (auto& x : indices) x += idx_offset;
			std::copy(indices, indices + 6, std::back_inserter(retval.indices));
		}
	}

	return retval;
}



std::unique_ptr<mk::Chunk> get_chunk_front_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 63; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z + 1ULL));
		}
		retval->voxels_set.at(y).at(63) = chunk.voxels_set.at(y).at(63);
	}

	if (chunk.neighbors[mk::FRONT]) {
		for (int y = 0; y < 64; ++y) {
			retval->voxels_set.at(y).at(63) &= ~chunk.neighbors[mk::FRONT]->voxels_set[y][0];
		}
	}
	else {
		for (int y = 0; y < 64; ++y) {
			//retval->voxels_set.at(y).at(63) = 0; // TODO: NOTE: This should be done in an actual game with
												 // dynamically loading chunks 
			//retval->voxels_set.at(y).at(63) = UINT64_MAX;
		}
	}

	return retval; 
}

std::unique_ptr<mk::Chunk> get_chunk_back_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 1; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z - 1ULL));
		}
		retval->voxels_set.at(y).at(0) = chunk.voxels_set.at(y).at(0);
	}

	if (chunk.neighbors[mk::BACK]) {
		for (int y = 0; y < 64; ++y) {
			retval->voxels_set.at(y).at(0) &= ~chunk.neighbors[mk::BACK]->voxels_set[y][63];
		}
	}
	else {
		for (int y = 0; y < 64; ++y) {
			//retval->voxels_set.at(y).at(0) = 0;
			//retval->voxels_set.at(y).at(0) = UINT64_MAX;
		}
	}

	return retval;
}

std::unique_ptr<mk::Chunk> get_chunk_left_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z) >> 1);
		}
	}

	if (chunk.neighbors[mk::LEFT]) {
		for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) {
			// check right-most voxel of each neighbor row 
			retval->voxels_set.at(y).at(z) &= ~((chunk.neighbors[mk::LEFT]->voxels_set.at(y).at(z) & 1ULL) << 63);
		}
	}
	else {
		for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) {
			//retval->voxels_set.at(y).at(z) &= ~(1ULL << 63);
			//retval->voxels_set.at(y).at(z) |= (1ULL << 63);
		}
	}

	return retval;
}

std::unique_ptr<mk::Chunk> get_chunk_right_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

	for (int y = 0; y < 64; ++y) {
		for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) = chunk.voxels_set.at(y).at(z) & ~(chunk.voxels_set.at(y).at(z) << 1);
		}
	}

	if (chunk.neighbors[mk::RIGHT]) {
		for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) {
			retval->voxels_set.at(y).at(z) &= ~((chunk.neighbors[mk::RIGHT]->voxels_set.at(y).at(z) & (1ULL << 63)) >> 63);
		}
	}
	else {
		for (int y = 0; y < 64; ++y) for (int z = 0; z < 64; ++z) {
			//retval->voxels_set.at(y).at(z) &= ~1ULL;
			//retval->voxels_set.at(y).at(z) |= 1ULL;
		}
	}

	return retval;
}

std::unique_ptr<mk::Chunk> get_chunk_top_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

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

std::unique_ptr<mk::Chunk> get_chunk_bottom_mask(const mk::Chunk& chunk) {
	auto retval = std::make_unique<mk::Chunk>();

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

// block chunk impl 

void mk::BlockChunk::set_block_type_at(glm::tvec3<uint8_t> pos, VoxelType type) {
	if (type == VoxelType::NONE) {
		for (int y = 0; y < 4; ++y) for (int z = 0; z < 4; ++z) {
			int yp = 4 * pos.y + y;
			int zp = 4 * pos.z + z;

			chunk->voxels_set.at(yp).at(zp) &= ~(0b1111ULL << (60 - (4 * pos.x)));
		}
	}
	else {
		for (int y = 0; y < 4; ++y) for (int z = 0; z < 4; ++z) {
			int yp = 4 * pos.y + y;
			int zp = 4 * pos.z + z;

			chunk->voxels_set.at(yp).at(zp) |= (0b1111ULL << (60 - (4 * pos.x)));
		}
	}

	for (int y = 0; y < 4; ++y) for (int z = 0; z < 4; ++z) for (int x = 0; x < 4; ++x) {
		int xp = 4 * pos.x + x;
		int yp = 4 * pos.y + y;
		int zp = 4 * pos.z + z;

		chunk->voxel_types->at(yp).at(zp).at(xp) = type;
	}
}

mk::VoxelType mk::BlockChunk::get_block_type_at(glm::tvec3<uint8_t> pos) {
	int xb = 4 * pos.x;
	int yb = 4 * pos.y;
	int zb = 4 * pos.z;

	return chunk->voxel_types->at(yb).at(zb).at(xb); // TODO: Will eventually need to evaluate more than just the anchor voxel 
}
