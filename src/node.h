#pragma once

#ifndef _DVSIM_NODE_H_
#define _DVSIM_NODE_H_

#include "types.h"
#include "util.h"
#include <unordered_map>

namespace DVSim {

class Node {
public:
	Node() { }

	Node(const NodeConfig& config);

	NodeName node_name() const { return node_name_; }

	uint16_t port() const { return port_; }

	// Pretty print of neighbors table
	void print_nbors_table() const;
	
	// Pretty print of distance vector table
	void print_dv_table() const;

	void start();
	
private:
	NodeName node_name_;
	uint16_t port_;
	std::unordered_map<NodeName, DistAndNextHop> dv_;
	std::unordered_map<NodeName, IPAndDist> nbors_;
};

}

#endif
