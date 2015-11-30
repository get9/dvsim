#pragma once

#ifndef _DVSIM_NODE_H_
#define _DVSIM_NODE_H_

#include <unordered_map>
#include "util.h"

namespace DVSim {

class Node {
public:
	Node() { }

	Node(const NodeConfig& config);

	NodeName node_name const { return node_name_; }

	uint16_t port const { return port_; }

	// Pretty print of neighbors table
	friend std::ostream& operator<<(std::ostream& s, const decltype(nbors_)& nbors)
	{
		s << "Neighbor Table" << std::endl;
		for (const auto& entry : nbors) {
			NodeName name;
			std::string ip;
			Distance d;
			std::tie(name, ip, d) = entry;
			s << name << "\t" << ip << "\t" << d << std::endl;
		}
		return s;
	}

	// Pretty print of distance vector table
	friend std::ostream& operator<<(std::ostream& s, const decltype(dv_)& dv)
	{
		s << "Distance Vector Table" << std::endl;
		for (const auto& entry : dv) {
			NodeName name;
			Distance d;
			NextHop next_hop;
			std::tie(name, d, next_hop) = entry;
			s << name << "\t" << d << "\t" << next_hop << std::endl;
		}
		return s;
	}

private:
	NodeName node_name_;
	uint16_t port_;
	std::unordered_map<NodeName, DistAndNextHop> dv_;
	std::unordered_map<NodeName, IPAndDist> nbors_;
};

}

#endif
