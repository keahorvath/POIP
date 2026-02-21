#ifndef HEUR_HPP
#define HEUR_HPP

#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

class Heuristic {
   public:
    Heuristic(const WarehouseSolution& initial_solution);
    virtual ~Heuristic() = default;

    WarehouseSolution solution;
    long long solution_cost;

    std::unordered_map<int, std::vector<int>> product_to_orders;
    std::vector<std::vector<int>> path;
    std::vector<std::vector<int>> circuit_products;
    std::vector<int> rack_used;
    std::vector<std::pair<int, int>> circuit_intervals;

    // Common methods to both heuristics

    /**
     * @brief Associate each product with the set of orders it is part of.
     */
    void buildProductToOrders();
    /**
     * @brief Build the vector indicating the number of products currently assigned to each rack.
     */
    void buildRackUsed();
    /**
     * @brief Determine the interval [min_rack, max_rack] for each circuit based on the current assignment.
     * This is used to limit the search space during the improvement phase.
     */
    void buildCircuitIntervals();

    /**
     * @brief Calculate the total cost of the current solution and build the path for each order.
     * The path for an order is the sequence of racks that need to be visited to pick all products in the order, starting from the entrance (rack 0)
     * and ending at the exit (rack num_racks-1).
     */
    void calculateCostAndPath();
};

#endif