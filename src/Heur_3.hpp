#ifndef HEUR_3_HPP
#define HEUR_3_HPP

#include <vector>
#include <unordered_map>
#include <utility> // for pair
#include <set>
#include <random> // for mt19937

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
};

vector<vector<int>> build_product_in_circuit(const WarehouseInstance& data);
vector<int> read_frequency_circuits(string filename, int num_circuits);
vector<int> new_rack_capacity(const WarehouseInstance& data);
unordered_map<int, vector<int>> read_product_pairs(const string& filename, int num_products);
vector<vector<int>> read_freq_prod(string filename, int num_circuit, vector<int> product_circuit);
vector<vector<int>> read_never_used_products(string filename, int num_circuit, int num_products, vector<int> product_circuit);


#endif 
