#include "node.h"
#include <iostream>

using namespace DVSim;

Node::Node(const NodeConfig& config) : node_name_(config.node_name)
{
	port_ = config.port;

	for (auto& nbor : config.neighbors) {
		NodeName nbor_name;
		Distance nbor_dist;
		std::string nbor_ip;
		std::tie(nbor_name, nbor_dist, nbor_ip) = nbor;
		
		// Add to neighbor and distance vector tables
		nbors_.emplace(nbor_name, IPAndDist(nbor_ip, nbor_dist));
		dv_.emplace(nbor_name, DistAndNextHop(nbor_dist, nbor_name));
	}
}

// Print neighbors table
void Node::print_nbors_table() const
{
	std::cout << "Neighbor Table" << std::endl;
	for (const auto& entry : nbors_) {
		NodeName name;
		std::string ip;
		Distance d;
		IPAndDist id;
		std::tie(name, id) = entry;
		std::tie(ip, d) = id;
		std::cout << name << "\t" << ip << "\t" << d << std::endl;
	}
}

// Print distance vector table
void Node::print_dv_table() const
{
	std::cout << "Distance Vector Table" << std::endl;
	for (const auto& entry : dv_) {
		NodeName name;
		DistAndNextHop dn;
		Distance d;
		NextHop next_hop;
		std::tie(name, dn) = entry;
		std::tie(d, next_hop) = dn;
		std::cout << name << "\t" << d << "\t" << next_hop << std::endl;
	}
}

void Node::start()
{
	
}
