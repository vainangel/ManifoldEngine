#include "IOUtil.hpp"

namespace fs = std::filesystem;

#include <iostream>
#include <fstream>
#include <sstream>

std::string mk::util::get_resource(std::filesystem::path path) {
	if (not fs::exists(path)) {
		std::cerr << "Resource '" << path.string() << "' could not be found.\n";
		return {};
	}

	std::ifstream ifs(path);
	std::ostringstream sstr;
	sstr << ifs.rdbuf();
	
	return sstr.str();
}