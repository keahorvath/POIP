#include "Heur_1.hpp"

using namespace std;

int calculate_cost(const WarehouseSolution& solution) {
    const auto& adj = solution.data.adjacency;
    int start = 0;
    int end = solution.data.num_racks - 1;

    int total = 0;

    for (const auto& order : solution.data.orders) {
        
        // Add racks
        set<int> racks_set;
        for (int product : order) {
            racks_set.insert(solution.assignment[product]);
        }

        // Build path
        std::vector<int> path;
        path.push_back(start);
        path.insert(path.end(), racks_set.begin(), racks_set.end());
        path.push_back(end);

        
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            total += adj[path[i]][path[i + 1]];
        }
    }

    return total;
}

WarehouseSolution Glouton (const WarehouseInstance& data, vector<vector<int>>& current_sequence){}

void swap (int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

void swap (vector<int>& a, vector<int>& b){
    vector<int> tmp = a;
    a = b;
    b = tmp;
}

void RL_products(const WarehouseInstance& data, WarehouseSolution& current_solution,
    vector<vector<int>>& current_sequence, bool& stop, int& current_cost) {

    for (int f = 0; f < data.num_circuits; f++) {
        for (int k = 0; k < (int)current_sequence[f].size() - 1; k++) {

            if (current_solution.assignment[current_sequence[f][k]]
                != current_solution.assignment[current_sequence[f][k+1]]) {

                swap(current_solution.assignment[current_sequence[f][k]],
                    current_solution.assignment[current_sequence[f][k+1]]);

                int new_cost = calculate_cost(current_solution);

                if (new_cost < current_cost) {
                    stop = false;
                    swap(current_sequence[f][k], current_sequence[f][k+1]);
                    current_cost = new_cost;
                } else {
                    swap(current_solution.assignment[current_sequence[f][k]],
                        current_solution.assignment[current_sequence[f][k+1]]);
                }
            }
        }
    }
}

void RL_circuits(const WarehouseInstance& data, WarehouseSolution& current_solution,
    vector<vector<int>>& current_sequence, bool& stop, int& current_cost){

        for (int f = 0; f < data.num_circuits - 1; f++) {
            swap(current_sequence[f], current_sequence[f+1]);
            int new_cost = calculate_cost (Glouton(data, current_sequence));

            if (new_cost < current_cost) {
                stop = false;
                current_solution = Glouton (data, current_sequence);
            } else {
                swap(current_sequence[f], current_sequence[f+1]);
            }
        }

}

WarehouseSolution Heur_1 (const WarehouseInstance& data, int iter_max) {

    vector<vector<int>> current_sequence (data.num_circuits, vector<int> (0));
    for (int j = 0; j < data.num_products; j++) {
        current_sequence[data.product_circuit[j]].push_back(j);
    }

    WarehouseSolution current_solution = Glouton (data, current_sequence);

    int current_cost = calculate_cost(current_solution);

    int iter = 0;
    bool stop = false;
    while (++iter < iter_max && !stop) {
        stop = true;
        RL_products(data, current_solution, current_sequence, stop, current_cost);
        RL_circuits(data, current_solution, current_sequence, stop, current_cost);
    }

    return current_solution;
}
