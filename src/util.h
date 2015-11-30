#include <vector>
#include <tuple>
#include <string>
#include <fstream>


using NeighborTriplet = std::tuple<std::string, int32_t, std::string>;

struct NodeConfig {
	std::string node_name;
	uint16_t port;
	std::vector<NeighborTriplet> neighbors;
};

// Parse the config file from input (either stdin or file)
NodeConfig parse_config_file(const std::ifstream& stream);

std::vector<std::string> split(const std::string& s, char delim);
