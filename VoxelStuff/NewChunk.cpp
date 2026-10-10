#include "NewChunk.hpp"

mk::pre::Chunk::Chunk(ChunkLocation location) : m_location{ location } {
	m_chunk_volume = std::make_shared<ChunkVolume>();
}

void mk::pre::Chunk::set_block(ChunkSpacePosition pos, VoxelType type) {

}

std::shared_ptr<mk::pre::Chunk> mk::pre::ChunkProvider::get_chunk(ChunkLocation location) {

}

void mk::pre::World::reorder_loaded_chunks_list(ChunkLocation center) {

}

void mk::pre::World::update_loaded_chunks(BlockLocation player_location) {

}

void mk::pre::WorldGenerator::on_chunk_first_load(Chunk& chunk) {

}