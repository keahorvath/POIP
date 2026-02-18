#ifndef HEUR_3_HPP
#define HEUR_3_HPP

#include <vector>
#include <unordered_map>
#include <utility> // for pair
#include <set>
#include <random> // for mt19937
#include <deque>
#include <climits>

#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

using namespace std;

class Heuristic_3 {
    public:
        Heuristic_3();
        Heuristic_3(const WarehouseSolution& initial_solution);

        void improve(const int max_attempts_per_circuit = 5000, const int stagnation_threshold = 5000);

        WarehouseSolution solution;
        long long solution_cost;

        void build_product_to_orders();
        void build_rack_used();
        void build_circuit_intervals();
        void calcul_cost_and_path();
        vector<int> initial_solution3(const vector<int>& frequency_circuits, vector<vector<int>> freq_products, unordered_map<int, vector<int>> product_pairs, vector<vector<int>> never_used);

        unordered_map<int, vector<int>> product_to_orders;
        vector<vector<int>> path;
        vector<vector<int>> circuit_products;
        vector<int> rack_used;
        vector<pair<int, int>> circuit_intervals;
        vector<vector<int>> rack_content;

        std::vector<int> rack_to_aisle;             // rack -> aisle id

        // --- Tabu search ---
        struct TabuMove {
            int r1;
            int r2;
        };

        std::deque<TabuMove> tabu_list;
        int tabu_tenure = 50; // valeur par défaut, recalculée par circuit

        // --- Best global (aspiration) ---
        long long best_cost = LLONG_MAX;

        // --- Tabu search tuning ---
        double proba_swap_aeration = 0.30;      // 30%
        double proba_accept_non_improving = 0.05; // si stagnation haute
        int stagnation_global = 0;


        // --- Random ---
        std::mt19937 rng;

        void build_rack_to_aisle();
        int find_in_rack(int rack, int product) const;
        bool is_tabu(int r1, int r2) const;
        void push_tabu(int r1, int r2);
        int find_empty_slot_in_rack(int rack) const;
        void improve3(const int max_attempt_per_circuit,const int stagnation_threshold, bool allow_aeration_swaps = false);
};

long long int factoriel(int n, long long cap);
vector<vector<int>> build_product_in_circuit(const WarehouseInstance& data);
vector<int> read_frequency_circuits(string filename, int num_circuits);
vector<int> new_rack_capacity(const WarehouseInstance& data);
unordered_map<int, vector<int>> read_product_pairs(const string& filename, int num_products);
vector<vector<int>> read_freq_prod(string filename, int num_circuit, vector<int> product_circuit);
vector<vector<int>> read_never_used_products(string filename, int num_circuit, int num_products, vector<int> product_circuit);
vector<vector<int>> read_concordance_circuit(const string& filename);
vector<int> build_circuit_order_by_concordance_fast_weighted(const vector<int>& frequency_circuits, const vector<vector<int>>& concord, const vector<int>& freq_count);
void read_frequency_circuits_and_counts(const string& filename, int num_circuits, vector<int>& frequency_circuits, vector<int>& freq_count);

#endif 
