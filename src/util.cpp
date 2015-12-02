#include "util.h"
#include <stdexcept>
#include <sstream>
#include <cassert>

/*
 * XXX: assumes well-formed file that looks like the following:
 * name_of_this_node
 * port_number
 * neighbor_1_name cost_of_the_link_to_this_neighbor neighbor_1_IP_address
 * neighbor_2_name cost_of_the_link_to_this_neighbor neighbor_2_IP_address
 * ....
 * neighbor_n_name cost_of_the_link_to_this_neighbor neighbor_n_IP_address
 */
NodeConfig parse_config_file(std::istream& stream)
{
	// Check if file is open, fail fast if not
	if (!stream) {
		throw std::runtime_error("[error]: cannot open file");
	}

	// Parse file
	NodeConfig config;
	std::string line;

	// Add node name
	std::getline(stream, line);
	config.node_name = line;

	// Add port
	std::getline(stream, line);
	config.port = uint16_t(std::stoi(line));

	// Add neighbor info
	while (std::getline(stream, line)) {
		auto split_contents = split(line, ' ');
		config.neighbors.emplace_back(
				split_contents[0],
				std::stoi(split_contents[1]),
				split_contents[2]);
	}

	return config;
}

// Split string 's' on 'delim' into elements
std::vector<std::string> split(const std::string& s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> items;
	while (std::getline(ss, item, delim)) {
		items.emplace_back(item);
	}
	return items;
}

// Deserialize a message from the wire
DVMessage deserialize(const std::string& msg)
{
	std::stringstream ss(msg);
	std::string line;

	// Name
	DVSim::NodeName name;
	std::getline(ss, name);

	// Count
	std::getline(ss, line);
	int32_t dst_count = std::stoi(line);

	// Entries
	std::vector<MsgTriplet> ents;
	for (int32_t i = 0; i < dst_count; ++i) {
		std::getline(ss, line);
		auto elems = split(line, ' ');
		assert(elems.size() == 3 && "elems.size() was not 3");
		ents.emplace_back(elems[0], std::stoi(elems[1]), elems[2]);
	}
	
	DVMessage dv;
	dv.sender = name;
	dv.entries = ents;
	return dv;
}
