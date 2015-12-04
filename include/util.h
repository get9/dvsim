#pragma once

#ifndef _DVSIM_UTIL_H_
#define _DVSIM_UTIL_H_

#include <vector>
#include <tuple>
#include <string>
#include <fstream>

#include "types.h"

constexpr int32_t kDefaultPeriodicSendDelayMs = 10000;

// Convenient typedef's for long types
typedef std::tuple<DVSim::NodeName, DVSim::Distance, std::string> NeighborTriplet;
typedef std::tuple<DVSim::NodeName, DVSim::Distance, DVSim::NextHop> MsgTriplet;

// Input config for a node
struct NodeConfig {
    DVSim::NodeName node_name;
    uint16_t port;
    std::vector<NeighborTriplet> neighbors;
};

// The message each node is sending
struct DVMessage {
    DVSim::NodeName sender;
    std::vector<MsgTriplet> entries;
};

// Parse the config file from input (either stdin or file)
NodeConfig parse_config_file(std::istream& stream);

std::vector<std::string> split(const std::string& s, char delim);

DVMessage deserialize(const std::string& msg);

#endif
