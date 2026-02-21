#ifndef HEUR_1_HPP
#define HEUR_1_HPP

#include <set>

#include "Utilitary.hpp"
#include "WarehouseLoader.hpp"
#include "WarehouseSolution.hpp"

int calculate_cost(const WarehouseSolution& solution);
std::vector<int> nbFreeLoc(const WarehouseInstance& data);
std::vector<int> New_rack_capacity(const WarehouseInstance& data);

WarehouseSolution glouton(const WarehouseInstance& data, std::vector<std::vector<int>>& current_sequence);

void swap(int& a, int& b);
void swap(std::vector<int>& a, std::vector<int>& b);

void RL_products(const WarehouseInstance& data, WarehouseSolution& current_solution, std::vector<std::vector<int>>& current_sequence, bool& stop,
                 int& current_cost);
void RL_circuits(const WarehouseInstance& data, WarehouseSolution& current_solution, std::vector<std::vector<int>>& current_sequence, bool& stop,
                 int& current_cost);

WarehouseSolution Heur_1(const WarehouseInstance& data, int iter_max);

#endif