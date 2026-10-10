#pragma once 

// deprecated, sticking with flat voxel grid with sublevels 
namespace mk {
	namespace oct {
		// octree-based voxel/chunk layout 
		// voxel (1 byte)
		// 0b 00000000 -> no children (empty) 
		// 0b 11111111 -> all children are set (full) 
		// 
		
		struct Octree {

		};

		struct Voxel { // just for annotation 

		};
		
		struct Chunk {
		
		};
	}
}