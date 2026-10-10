#pragma once 

// draft header for new voxel format 

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

#include <cstddef>
#include <cstdint>



// DONT OVER-OPTIMIZE. MAKE SOMETHING THAT WORKS. IMPROVE IT LATER.



namespace mk {
	// draft namespace to avoid naming conflicts 
	namespace pre {
		struct BlockLocation {
			int64_t x;
			int64_t y;
			int64_t z;
		};
		using ChunkLocation = BlockLocation;

		enum class VoxelType {
			NONE = 0,
			STONE = 1,
			DIRT = 2,
			GRASS = 3
		};

		class Sublevel { // NOTE: May replace with octree 
			uint16_t voxels_set[16][16];
			VoxelType type; // will only support one voxel type for now. not permanent. 
		}; // NOTE: During meshing, can cache sublevel meshes to avoid re-reading when re-meshing 

		enum class BlockFace {
			TOP = 0,	// +y
			BOTTOM = 1, // -y
			LEFT = 2,	// -x
			RIGHT = 3,	// +x
			FRONT = 4,	// +z 
			BACK = 5	// -z 
		};

		// Considering using tall, 2D chunks, and scaling the vertex size accordingly....

		constexpr size_t WORLD_HEIGHT = 256;

		/// <summary>
		/// The composition of a voxel chunk. 
		/// NOTE: May replace voxel type with a dynamic voxel ID system with mappings
		/// </summary>
		struct ChunkVolume {
			// Mask of set, full blocks
			// Mapped by xs[y][z] & (0x80000000 >> (32ULL - x))
			uint32_t full_voxels_set[WORLD_HEIGHT][32];

			// Material of each block. If `(x, y, z)` would be a sublevel block,
			// then `voxel_type_at[y][z][x]` is the main material of the block. 
			VoxelType voxel_type_at[WORLD_HEIGHT][32][32];

			using LocationHashType = uint64_t;
			// Locations which utilize sublevels. I cannot see this performing well. Sigh.
			std::unordered_map<LocationHashType, Sublevel> sublevels;
		};

		struct ChunkSpacePosition {
			int x; int y; int z;
		};

		/// <summary>
		/// A ChunkContext contains composition information about a chunk, entities within the chunk,
		/// and other chunk-specific data.
		/// </summary>
		class Chunk {
			std::shared_ptr<ChunkVolume> m_chunk_volume; 
			ChunkLocation m_location;
		public:
			Chunk(ChunkLocation location);

			void set_block(ChunkSpacePosition pos, VoxelType type);
		};

		

		/// <summary>
		/// Handles loading and caching of chunks. This is a singleton class.
		/// </summary>
		class ChunkProvider {
		private:
			ChunkProvider();
			~ChunkProvider() = default;
		public:
			/// <summary>
			/// Retrieve a chunk at the given chunk `location`. This function attempts to find the chunk
			/// in memory before loading it from storage, and will allocate a new chunk if it is not found.
			/// This is independent from world generation. World generation will invoke the ChunkProvider 
			/// when generating new chunks. This function only retrieves chunks.
			/// </summary>
			/// <param name="location">Chunk location of the given chunk. Defined as world space location divided by chunk width.</param>
			/// <returns>A reference to a chunk at `location`.</returns>
			static std::shared_ptr<Chunk> get_chunk(ChunkLocation location);
		};

		

		/// <summary>
		/// A Region describes a group of chunks. In draft. May be used to group chunks together for
		/// I/O and caching operations.
		/// </summary>
		class Region {};

		// -- Grouping chunks based on their distance from the player
		// 
		// Rationale: When the player is moving around the world, 
		// chunks farther away from the player are increasingly likely
		// to go in-and-out of render distance. Thus, searching for chunks
		// at the edge of the player's render distance is necessary when
		// deciding what chunks to load and unload as the player moves. This
		// creates a necessity to quickly search for chunks by their euclidian
		// distance from the player, which is computationally tedious when 
		// using unordered or hashed lists to store geometric information.
		// 
		// Solution:
		// Store the initial grouping of chunks by their distance from the player,
		// e.g., at the end of list. When the player moves, iterate the list until
		// the current chunk is within render distance, and cache/cull any chunks
		// which have been iterated. Next, iterate the list of chunks until all of their
		// furthest neighbors (from directions away from the player) are no longer in render
		// distance, loading any chunks that do not exist in the list (a redundant hashmap
		// may be used for random chunk lookup) and adding them to the end of the list, sorting
		// each time to ensure list ordering remains consistent. 
		// 
		// Performance/complexity:
		// The initial sorting of the list will take on average o(n log n) for n chunks in 
		// render distance, but each consecutive reordering of items in the list may be
		// o(2sqrt(n)) depending on the proximity algorithm used to determine distance
		// (2sqrt(x) refers to the possible size of the render distance perimeter, though it
		// if the render distance is circular, then it's defined by circumference). In the case
		// of typical movements made by a player, the time to re-sort the list is similar to the
		// non-asymptotic distance: omega(sqrt(n)).
		// 
		// The worst-case performance of this operation is O(n log n), i.e., the initial run of
		// the algorithm, as well as in the case that the player's location changes drastically. 
		// 
		// -- Chunk loading and caching 
		// 
		// As chunks load and unload, some are cached in memory. Chunks neighboring loaded chunks
		// are kept in memory for the sake of meshing, but will not be simulated nor rendered. 
		// Unloaded chunks are marked for destruction when they do not neighbor any loaded chunk, and 
		// are asynchronously written to a file and destroyed. To avoid race conditions, chunks are
		// read from memory before they're loaded from a file, and will not be destroyed until file
		// writing is complete. During early debugging, the world will simply remain in memory until
		// the program ends. World writing and loading will be one of the last things added to the 
		// initial world system. 
		// 
		// -- General chunk loading
		// 
		// Loading all chunks in an area [x, z] requires computing every pair of chunk coordinates in
		// the render distance of the player, i.e., all combinations of (x, z) for (-x->x, -z->z), offset
		// by player chunk location. 
		// 
		// Because chunk meshing partly relies on reading neighboring chunk volumes, the world will load
		// all chunks within the render distance of the player plus one chunk, keeping the edge chunks
		// in memory but unrendered. 
		// 

		struct WorldSettings {}; // may make polymorphic for extensibility 

		class ClientSettings { // may make polymorphic for extensibility 
		public:
			size_t render_distance;
		};

		class WorldGenerator;

		/// <summary>
		/// A World describes the universe of chunks in a given instance.
		/// </summary>
		class World {
			std::vector<std::shared_ptr<Chunk>> m_loaded_chunks; // ordering furthest away last for easy resizing 
			
			// Sort m_loaded_chunks by increasing order of distance from `center`. 
			void reorder_loaded_chunks_list(ChunkLocation center);

		public:
			std::shared_ptr<ClientSettings> client_settings;
			std::shared_ptr<WorldGenerator> generator; // procedural generation context 
			std::string name;
		
			/// <summary>
			/// Update the loaded world based on the specified player location. 
			/// </summary>
			/// <param name="player_location"></param>
			void update_loaded_chunks(BlockLocation player_location);
		};

		class WorldGenerator { // for now, just make a world that generates volume and save to memory on unload
			World& m_this_world;
		public:
			virtual void on_chunk_first_load(Chunk& chunk);

		};
	}
}