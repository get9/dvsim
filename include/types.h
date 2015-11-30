#pragma once

#ifndef _DVSIM_TYPES_H_
#define _DVSIM_TYPES_H_

#include <tuple>
#include <string>

namespace DVSim {

using Distance  	 = int32_t;
using NodeName       = std::string;
using NextHop   	 = std::string;
using IPAndDist      = std::pair<std::string, Distance>;
using DistAndNextHop = std::pair<Distance, NextHop>;

}

#endif
