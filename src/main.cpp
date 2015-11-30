#include "util.h"
#include "node.h"

#include <iostream>
#include <cstdlib>


int main(int argc, char** argv)
{
	// CLI argument verification
	if (argc < 2) {
		std::cerr << "Usage:" << std::endl;
		std::cerr << "    " << argv[0] << " config.txt" << std::endl;
		std::exit(1);
	}

	// Parse config file (either from stdin or from file)
	NodeConfig config = parse_config_file(instream);
	if (argv[2] == "<") {
		config = parse_config_file(std::cin);
	} else {
		std::ifstream infile(argv[2]);
		config = parse_config_file(infile);
	}

	// Start a node
	DVSim::Node this_node(config);
	this_node.start();
}
