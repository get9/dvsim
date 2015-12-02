#pragma once

#ifndef _DVSIM_NODE_H_
#define _DVSIM_NODE_H_

#include "types.h"
#include "util.h"
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace DVSim {

class Node {
public:
	Node() { }

	Node(const NodeConfig& config);

	NodeName node_name() const { return node_name_; }

	uint16_t port() const { return port_; }

	// Pretty print of neighbors table
	void print_nbors_table();
	
	// Pretty print of distance vector table
	void print_dv_table();

	// Main entry point to algorithm. Does not return
	void start();

	// Periodically send update to neighbors
	void periodic_send(int32_t interval_ms=kDefaultPeriodicSendDelayMs);

	// Send update to all neighbors
	void nbor_broadcast();

	// Send message to IP address
	void send_message(const std::string& ip_addr, const std::string& msg);

	// Receive incoming connection
	int32_t receive_connection();

	// Receive the message from connection
	std::string receive_message(int32_t conn);

	// Update the tables accordingly
	bool update_dv_table(const DVMessage& msg);
	
private:
	NodeName node_name_;
	uint16_t port_;

	// Boolean that tells whether threads should interrupt
	//std::atomic<bool> should_interrupt_;

	// Tables and their associated mutex
	std::unordered_map<NodeName, DistAndNextHop> dv_;
	std::unordered_map<NodeName, IPAndDist> nbors_;
	std::mutex table_mutex_;

	// Creates a message to send to neighbors
	std::string create_message();
};

}

#endif
