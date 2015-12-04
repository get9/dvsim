#pragma once

#ifndef _DVSIM_TYPES_H_
#define _DVSIM_TYPES_H_

#include <tuple>
#include <string>

namespace DVSim
{

typedef int32_t Distance;
typedef std::string NodeName;
typedef std::string NextHop;
typedef std::pair<std::string, Distance> IPAndDist;
typedef std::pair<Distance, NextHop> DistAndNextHop;
}

#endif
