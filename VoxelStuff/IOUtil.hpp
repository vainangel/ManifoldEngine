#pragma once 

#include <filesystem>
#include <string>

namespace mk::util {
	std::string get_resource(std::filesystem::path);
}