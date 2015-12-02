#include "node.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <mutex>
#include <tuple>
#include <cstring>
#include <cerrno>
#include <cstdio>
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
		nbors_.insert(std::make_pair(
					nbor_name, std::move(IPAndDist(nbor_ip, nbor_dist))));
		dv_.insert(std::make_pair(
					nbor_name, std::move(DistAndNextHop(nbor_dist, nbor_name))));
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
				std::cout << "periodic send to neighbors" << std::endl;
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
		send_message(ip_addr, msg);
	}
}

// Send a msg to the specified ip_addr
void Node::send_message(const std::string& ip_addr, const std::string& msg)
{
	// Networking code to set up address/socket
    struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	std::string port_str = std::to_string(port_);
    struct addrinfo *server_info;
    int err = getaddrinfo(ip_addr.c_str(), port_str.c_str(), &hints, &server_info);
    if (err != 0) {
		std::cerr << "[get_addr_por]: getaddrinfo "
			      << gai_strerror(err) << std::endl;
		return;
    }

    // Loop through to find socket to bind to
	int sock;
    struct addrinfo *p;
    for (p = server_info; p != NULL; p = p->ai_next) {

		// Get a socket
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) {
            perror("[get_addr_port]: socket");
            continue;
        }

		// Connect to it
		if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock);
			std::cerr << "[send_message]: connect (" << ip_addr << ")" << std::endl;
			continue;
		}
        break;
    }

    // Did we bind to a socket?
    if (p == NULL) {
		std::cerr << "[get_addr_port]: could not get socket" << std::endl;
		return;
    }
    freeaddrinfo(server_info);

	// Add msg len to the beginning of the msg (+1 for null terminator)
	uint32_t msg_size = uint32_t(msg.size() + 1);
	uint32_t buf_size = msg_size + uint32_t(sizeof(msg_size));
	auto buf = std::unique_ptr<char>(new char[buf_size]);
	uint32_t net_order_msg_size = htonl(msg_size);
	std::memcpy(buf.get(), &net_order_msg_size, sizeof(net_order_msg_size));
	std::strncpy(buf.get() + sizeof(net_order_msg_size), msg.c_str(), msg_size);
	printf("buf = %s\n", buf.get());

	// Send (and make sure all data gets sent)
	ssize_t send_len = send(sock, buf.get(), buf_size, 0);
	if (send_len == -1) {
		perror("[send_message]: send");
		return;
	}
	while (send_len < buf_size) {
		send_len += send(sock, buf.get() + send_len,
				         buf_size - uint32_t(send_len), 0);
	}
}

// Main entry point to start the algorithm
void Node::start()
{
	// Setup listener
	int32_t listener_sock = set_listen_for_connections();
	if (listener_sock == -1) {
		std::cerr << "[start]: could not listen for connections" << std::endl;
		std::exit(1);
	}

	// Start the periodic send
	std::cout << "Initial routing table" << std::endl;
	print_dv_table();
	std::cout << std::endl;

	// Wait for all nodes to start listening before sending messages
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	periodic_send();

	while (true) {
		// receive_connection() is blocking!
		int32_t newsock = receive_connection(listener_sock);
		if (newsock == -1) {
			std::cerr << "[start]: couldn't receive connection" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		// Spawn thread to handle responses
		std::thread([this, newsock]() {
			std::string serialized_msg = this->receive_message(newsock);
			DVMessage msg = deserialize(serialized_msg);
			std::cout << "received message from " << msg.sender << std::endl;
			std::cout << serialized_msg << std::endl;
			if (this->update_dv_table(msg)) {
				this->nbor_broadcast();
			}
		}).detach();
	}
}

int32_t Node::set_listen_for_connections()
{
	// Starting to connect to socket for receiving
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	std::string port_str = std::to_string(port_);
    struct addrinfo *server_info;
    int err = getaddrinfo(nullptr, port_str.c_str(), &hints, &server_info);
    if (err != 0) {
		std::cerr << "[get_addr_por]: getaddrinfo "
			      << gai_strerror(err) << std::endl;
		return -1;
    }

	// Loop through until we find a socket to bind to
	struct addrinfo* p;
	int sock;
	for(p = server_info; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("[receive_connection]: socket");
            continue;
        }

		// Fix the "port unavailable" message
		int32_t yes = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1) {
            perror("[receive_connection]: setsockopt");
			return -1;
        }

		// bind to the socket
        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("[receive_connection]: bind");
            continue;
        }
        break;
    }

	freeaddrinfo(server_info);

	// Check if we actually bound to a socket
	if (p == NULL) {
		std::cerr << "[receive_connection]: could not bind to receiver socket"
			      << std::endl;
		return -1;
	}

	// Listen on socket
	if (listen(sock, 10) != 0) {
		perror("[receive_connection]: listen");
		return -1;
	}
	
	return sock;
}

int32_t Node::receive_connection(int32_t sock)
{
	// Accept connection on that socket
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	int newsock = accept(sock, (struct sockaddr*) &their_addr, &sin_size);
	if (newsock == -1) {
		perror("[receive_connection]: accept");
		return -1;
	}

	return newsock;
}

// Receive message from node connected through conn
std::string Node::receive_message(int32_t conn)
{
	// Receive first sizeof(int32_t) bytes for message size
	uint32_t msg_size;
	uint32_t bytes_recvd = 0;
	while (bytes_recvd < sizeof(msg_size)) {
		int32_t bytecount = (int32_t) recv(
				conn, (uint32_t *)&msg_size + bytes_recvd, sizeof(msg_size), 0);
		if (bytecount == -1) {
			perror("[receive_message]: recv");
			continue;
		}
		bytes_recvd += uint32_t(bytecount);
	}
	msg_size = ntohl(msg_size);

	// Receive the next msg_size bytes that contain the message
	// XXX Need + 1 for null terminator?
	auto buf = std::unique_ptr<char>(new char[msg_size]);
	bytes_recvd = 0;
	while (bytes_recvd < msg_size) {
		int32_t bytecount = (int32_t) recv(
				conn, buf.get() + bytes_recvd, msg_size, 0);
		if (bytecount == -1) {
			perror("[receive_message]: recv");
			continue;
		}
		bytes_recvd += uint32_t(bytecount);
	}

	// Returns copy of buf
	return std::string(buf.get());
}

bool Node::update_dv_table(const DVMessage& msg)
{
	// Lock access while messing with tables
	std::lock_guard<std::mutex> lg(table_mutex_);
	bool did_update_table = false;

	for (const auto& triplet : msg.entries) {
		NodeName node;
		Distance sender_to_node_dist;
		NextHop node_nh;
		std::tie(node, sender_to_node_dist, node_nh) = triplet;

		// Can skip if this is our node
		if (node == node_name_) {
			continue;
		}

		// Distance to sender (neighbor)
		else if (dv_.find(msg.sender) == std::end(dv_)) {
			std::cerr << "[update_tables]: sender " << msg.sender
				      << " not in dv table" << std::endl;
			continue;
		}
		Distance dist_to_sender = std::get<0>(dv_[msg.sender]);

		// node is not currently in the table - make a new entry
		if (dv_.find(node) == std::end(dv_)) {
			// Distance will be dist to sender + distance provided by sender
			DistAndNextHop dnh =
				DistAndNextHop(dist_to_sender + sender_to_node_dist, msg.sender);
			dv_.insert(std::make_pair(node, std::move(dnh)));
			did_update_table = true;
		}

		// otherwise node is in table - update if necessary
		Distance recv_to_node_dist;
		NextHop recv_to_node_nh;
		std::tie(recv_to_node_dist, recv_to_node_nh) = dv_[node];

		// Only update if dist to sender + sender to node less than what we've
		// already got
		if (dist_to_sender + sender_to_node_dist < recv_to_node_dist) {
			dv_[node].first = dist_to_sender + sender_to_node_dist;
			dv_[node].second = msg.sender;
			did_update_table = true;
		}
	}

	return did_update_table;
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
 *
 * Puts a \r\n\r\n at the end of the message
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
		ss << n << " " << d << " " << nh << std::endl;
	}
	return ss.str();
}
