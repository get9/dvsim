#pragma once

#ifndef _DVSIM_UTIL_H_
#define _DVSIM_UTIL_H_

#include <vector>
#include <tuple>
#include <string>
#include <fstream>

#include "types.h"

constexpr int32_t kDefaultPeriodicSendDelayMs = 10000;

// Neighbor config of the form: (Name, Distance, IP Addr)
using NeighborTriplet = std::tuple<DVSim::NodeName, DVSim::Distance, std::string>;

struct NodeConfig {
	DVSim::NodeName node_name;
	uint16_t port;
	std::vector<NeighborTriplet> neighbors;
};

// Parse the config file from input (either stdin or file)
NodeConfig parse_config_file(std::istream& stream);

std::vector<std::string> split(const std::string& s, char delim);

#endif
