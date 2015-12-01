#include "node.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <mutex>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace DVSim;

Node::Node(const NodeConfig& config) :
	node_name_(config.node_name), port_(config.port)//, should_interrupt_(false)
{
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

// Print neighbors table. Need to lock access to tables before printing.
void Node::print_nbors_table()
{
	std::cout << "Neighbor Table" << std::endl;
	std::unique_lock<std::mutex> lg(table_mutex_);
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

// Print distance vector table. Need to lock access to tables before printing.
void Node::print_dv_table()
{
	std::cout << "Distance Vector Table" << std::endl;
	std::unique_lock<std::mutex> lg(table_mutex_);
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

// Procedure that spawns a new thread and periodically sends current distance
// vector table to neighbors
void Node::periodic_send(int32_t interval_ms)
{
	std::thread([this, interval_ms]() {
			while (true) {
				this->nbor_broadcast();
				std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
			}
		}).detach();
}

// Sends update to neighbor. Note that it must lock the two tables before using
// them
void Node::nbor_broadcast()
{
	// Lock tables before we do anything
	std::unique_lock<std::mutex> lock(table_mutex_);
	std::string msg = create_message();

	std::vector<std::pair<NodeName, std::string>> nbors;
	for (const auto nbor : nbors_) {
		nbors.emplace_back(nbor.first, nbor.second.first);
	}
	// Done with critical section stuff
	lock.unlock();

	// Send messages to neighbors
	for (const auto& pair : nbors) {
		NodeName name;
		std::string ip_addr;
		std::tie(name, ip_addr) = pair;

		std::cout << msg << std::endl;
		send_message(ip_addr, msg);
	}
}

// Send a msg to the specified ip_addr
void Node::send_message(const std::string& ip_addr, const std::string& msg)
{
	// Get local machine info via getaddrinfo syscall
    struct addrinfo hints;
    struct addrinfo *llinfo;
	std::memset(&hints, 0, sizeof(hints)); 
    hints = (struct addrinfo) {
        .ai_family = AF_UNSPEC,
        .ai_protocol = IPPROTO_TCP,
        .ai_socktype = SOCK_STREAM
    };
	std::string port_str = std::to_string(port_);
    int status = getaddrinfo(ip_addr.c_str(), port_str.c_str(), &hints, &llinfo);
    if (status != 0) {
		throw std::runtime_error("send(): getaddrinfo: failed");
    }
    
    // Create socket for incoming connections. Must loop through linked list
    // returned by getaddrinfo and try to bind to the first available result
    struct addrinfo *s = NULL;
    int sock = 0;
    for (s = llinfo; s != NULL; s = s->ai_next) {
        // Connect to the socket
        sock = socket(s->ai_family, s->ai_socktype, s->ai_protocol);
        if (sock == -1) {
			std::cerr << "send(): socket: couldn't get socket" << std::endl;
			continue;
        }

		// Now connect()
		if (connect(sock, s->ai_addr, s->ai_addrlen) != 0) {
			std::cerr << "send(): connect: couldn't connect" << std::endl;
			continue;
		}
        break;
    }

    // Check that we didn't iterate through the entire getaddrinfo linked list
    // and clean up getaddrinfo alloc
    if (s == NULL) {
		throw std::runtime_error("send(): couldn't bind to any addresses");
    }
    freeaddrinfo(llinfo);

	// Add msg len to the beginning of the msg
	uint32_t msg_size = uint32_t(msg.size());
	uint32_t buf_size = msg_size + sizeof(msg_size);
	char* buf = new char[buf_size];
	uint32_t net_order_msg_size = htonl(msg_size);
	std::memcpy(buf, &net_order_msg_size, sizeof(net_order_msg_size));
	std::strncpy(buf + sizeof(net_order_msg_size), msg.c_str(), msg_size);

	// Send (and make sure all data gets sent)
	ssize_t send_len = send(sock, buf, buf_size, 0);
	if (send_len != 0) {
		throw std::runtime_error("send(): send: couldn't send data");
	}
	while (send_len < buf_size) {
		send_len += send(sock, buf + send_len, buf_size - uint32_t(send_len), 0);
	}
}

// Main entry point to start the algorithm
void Node::start()
{
	// Start the periodic send
	periodic_send();

	do {
		;
	} while (true);
}

/* 
 * XXX Assumes tables have already been locked
 * Serializes the dv_ table into the expected format:
 * 	   sender_name
 * 	   number_of_destinations
 * 	   dest_1_name dist_1
 * 	   dest_2_name dist_2
 * 	   dest_3_name dist_3
 * 	   dest_4_name dist_4
 * 	   dest_5_name dist_5
 */
std::string Node::create_message()
{
	std::string msg;
	std::stringstream ss(msg);
	ss << node_name_ << std::endl;
	ss << dv_.size() << std::endl;
	for (const auto& dest : dv_) {
		NodeName n;
		DistAndNextHop dnh;
		Distance d;
		NextHop nh;
		std::tie(n, dnh) = dest;
		std::tie(d, nh) = dnh;
		ss << n << " " << d << "\r\n\r\n";
	}
	return ss.str();
}
