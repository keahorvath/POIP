#pragma once

#include "WarehouseLoader.hpp"
#include "WarehouseSolution.hpp"

#include <set>

int calculate_cost(WarehouseSolution& solution);

WarehouseSolution Glouton (vector<vector<int>>& current_sequence);



WarehouseSolution Heur_1 (const WarehouseInstance& data);