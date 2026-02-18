#ifndef HEUR_2_HPP
#define HEUR_2_HPP

#include <vector>
#include <unordered_map>
#include <utility> // for pair
#include <set>
#include <random> // for mt19937

#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

using namespace std;

class Heuristic_2 {
    public:
        Heuristic_2();
        Heuristic_2(const WarehouseSolution& initial_solution);

        void improve(const int max_attempt_per_circuit = 5000, const int stagnation_threshold = 5000);

        WarehouseSolution solution;
        long long solution_cost;

        void build_product_to_orders();
        void build_rack_used();
        void build_circuit_intervals();
        void calcul_cost_and_path();

        unordered_map<int, vector<int>> product_to_orders;
        vector<vector<int>> path;
        vector<vector<int>> circuit_products;
        vector<int> rack_used;
        vector<pair<int, int>> circuit_intervals;
};

long long int factorial(int n, long long cap) ;
vector<vector<int>> build_product_in_circuit(const WarehouseInstance& data);
vector<int> read_frequency_circuits(string filename, int num_circuits);
vector<int> new_rack_capacity(const WarehouseInstance& data);
vector<int> initial_solution(const WarehouseInstance& data, const vector<int>& frequency_circuits);

#endif // HEUR_2_HPP
