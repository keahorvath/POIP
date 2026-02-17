#pragma once

//#include "gurobi_c++.h"
#include "unordered_set"

#include "WarehouseInstance.hpp"

std::vector<std::vector<int>> init_length (const WarehouseInstance& data);

std::vector<int> ordre_circuits (const WarehouseInstance& data);
