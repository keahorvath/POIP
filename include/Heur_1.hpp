#pragma once

#include "WarehouseLoader.hpp"
#include "WarehouseSolution.hpp"

#include <set>

using namespace std;

int calculate_cost(WarehouseSolution& solution);

WarehouseSolution Glouton (const WarehouseInstance& data, vector<vector<int>>& current_sequence);



void swap (int& a, int& b);

void swap (vector<int>& a, vector<int>& b);
    
void RL_products(const WarehouseInstance& data, WarehouseSolution& current_solution,
    vector<vector<int>>& current_sequence, bool& stop, int& current_cost);

void RL_circuits(const WarehouseInstance& data, WarehouseSolution& current_solution,
    vector<vector<int>>& current_sequence, bool& stop, int& current_cost);

WarehouseSolution Heur_1 (const WarehouseInstance& data, int iter_max);
