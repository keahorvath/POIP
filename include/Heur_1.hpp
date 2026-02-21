#ifndef HEUR_1_HPP
#define HEUR_1_HPP

#include <set>

#include "Utilitary.hpp"
#include "WarehouseLoader.hpp"
#include "WarehouseSolution.hpp"

/**
 * @brief Generates a warehouse solution using a greedy assignment strategy based on a circuit sequence.
 */
WarehouseSolution glouton(const WarehouseInstance& data, std::vector<std::vector<int>>& current_sequence);

/**
 * @brief Performs Local Search (RL) by swapping products within the same circuit to minimize cost.
 */
void RL_products(const WarehouseInstance& data, WarehouseSolution& current_solution, std::vector<std::vector<int>>& current_sequence, bool& stop,
                 int& current_cost);

/**
 * @brief Performs Local Search (RL) by reordering the sequence of circuits to improve global travel distance.
 */
void RL_circuits(const WarehouseInstance& data, WarehouseSolution& current_solution, std::vector<std::vector<int>>& current_sequence, bool& stop,
                 int& current_cost);

/**
 * @brief Heuristic 1: iteratively combines greedy construction and local search improvements.
 */
WarehouseSolution Heur_1(const WarehouseInstance& data, int iter_max);

#endif