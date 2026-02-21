#ifndef HEUR_3_HPP
#define HEUR_3_HPP

#include <climits>
#include <deque>
#include <random>  // for mt19937
#include <set>
#include <unordered_map>
#include <utility>  // for pair
#include <vector>

#include "Heur.hpp"
#include "Utilitary.hpp"
#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

class Heuristic_3 : public Heuristic {
   public:
    using Heuristic::Heuristic;  // Use parent constructor
    std::vector<std::vector<int>> rack_content;

    /**
     * @brief Builds an initial solution by first placing aeration in the left racks of each aisle, then filling the remaining space with products
     * from the most frequent circuits, while trying to group frequently paired products together. The never_used products are placed in the left
     * racks first, and any leftovers are placed in the remaining space after the frequent products. The placement is done in a way to ensure a good
     * mix of circuits in each aisle.
     */
    std::vector<int> buildInitialSolution(const std::vector<int>& frequency_circuits, std::vector<std::vector<int>> freq_products,
                                          std::unordered_map<int, std::vector<int>> product_pairs, std::vector<std::vector<int>> never_used);

    /**
     * @brief Improves the current solution by doing a local search based on tabu method, circuit by circuit
     */
    void improve(const int max_attempt_per_circuit, const int stagnation_threshold, bool allow_aeration_swaps = false);

   private:
    std::vector<int> rack_to_aisle;  // rack -> aisle id

    // Tabu search
    struct TabuMove {
        int r1;
        int r2;
    };

    std::deque<TabuMove> tabu_list;
    int tabu_tenure = 50;  // default, recalculated by circuit

    // Best global (aspiration)
    long long best_cost = LLONG_MAX;

    // Tabu search tuning
    double proba_swap_aeration = 0.30;         // 30%
    double proba_accept_non_improving = 0.05;  // if high stagnation
    int stagnation_global = 0;

    std::mt19937 rng;

    // METHODS USED TO CREATE THE INITIAL SOLUTION
    std::vector<int> calculateAisleAeration();
    void placeInitialAeration(std::vector<int>& aisle_aeration);
    void fillWarehouseForCircuit(int circuit, std::vector<int>& freqs, std::vector<int>& nus, std::unordered_map<int, std::vector<int>>& pairs,
                                 std::vector<bool>& used);
    bool fillEvenRack(std::vector<int>& freqs, std::vector<int>& nus, std::unordered_map<int, std::vector<int>>& pairs, int rack,
                      std::vector<bool>& used);
    bool fillOddRack(std::vector<int>& freqs, std::vector<int>& nus, std::unordered_map<int, std::vector<int>>& pairs, int rack,
                     std::vector<bool>& used);
    int popNextValid(std::vector<int>& pile, std::vector<bool>& used);
    void placeProduct(int p, int rack, std::vector<bool>& used);
    bool isCircuitEmpty(const std::vector<int>& freqs, const std::vector<int>& nus, const std::vector<bool>& used);
    bool rackHasSpace(int rack);

    // METHODS USED IN THE IMPROVEMENT PHASE
    void buildRackToAisle();
    int findInRack(int rack, int product) const;
    bool isTabu(int r1, int r2) const;
    void pushTabu(int r1, int r2);
    int findEmptySlotInRack(int rack) const;
    bool generateMove(int c, int min_r, int max_r, int interval_size, bool allow_aeration_swaps, std::uniform_real_distribution<double>& uni01,
                      int& p1, int& p2, int& r1, int& r2, bool& is_swap_with_empty);
    long long calculateDelta(int p1, int p2, int r1, int r2, bool is_swap_with_empty, const std::set<int>& affected_orders_set);
    void applyAndCommitMove(int p1, int p2, int r1, int r2, bool is_swap_with_empty, const std::set<int>& affected_orders_set, long long delta);
    std::set<int> getAffectedOrders(int p1, int p2, bool is_swap_with_empty);
};

#endif
