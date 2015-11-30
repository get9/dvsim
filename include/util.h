#include <vector>
#include <tuple>
#include <string>
#include <fstream>

#include "types.h"


// Neighbor config of the form: (Name, Distance, IP Addr)
using NeighborTriplet = std::tuple<DVSim::NodeName, DVSim::Distance, std::string>;

struct NodeConfig {
	NodeName node_name;
	uint16_t port;
	std::vector<NeighborTriplet> neighbors;
};

// Parse the config file from input (either stdin or file)
NodeConfig parse_config_file(const std::ifstream& stream);

std::vector<std::string> split(const std::string& s, char delim);
