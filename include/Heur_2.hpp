#ifndef HEUR_2_HPP
#define HEUR_2_HPP

#include <random>  // for mt19937
#include <set>
#include <unordered_map>
#include <utility>  // for pair
#include <vector>

#include "Heur.hpp"
#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

class Heuristic_2 : public Heuristic {
   public:
    using Heuristic::Heuristic;  // Use parent constructor

    /**
     * @brief Intra-circuit improvement heuristic: local search by swapping two products within the interval of racks of their circuit. Only the costs
     * of the affected orders are recalculated to evaluate the delta.
     */
    void improve(int max_attempts = 5000, int stagnation = 5000);

   private:
    // METHODS USED IN THE IMPROVEMENT PHASE
    /**
     * @brief Select two distinct random products from the given circuit. Returns false if a valid pair cannot be selected (e.g., all products are in
     * the same rack).
     */
    bool selectSwapCandidates(int circuit_idx, int& p1, int& p2);

    /**
     * @brief Get the set of orders affected by swapping products p1 and p2.
     */
    std::set<int> getAffectedOrders(int p1, int p2);

    /**
     * @brief Calculate the delta cost of swapping products p1 and p2 by only recalculating the costs of the affected orders.
     */
    long long calculateDelta(int p1, int p2, const std::set<int>& affected_orders);

    /**
     * @brief Apply the swap of products p1 and p2 and update the solution and paths of affected orders accordingly.
     */
    void applySwap(int p1, int p2, const std::set<int>& affected_orders);
};

/**
 * @brief Builds an initial solution by assigning products from the most frequent circuits to the racks closest to the exit.
 */
std::vector<int> buildInitialSolution(const WarehouseInstance& data, const std::vector<int>& frequency_circuits);

#endif