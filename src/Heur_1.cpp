#include "Heur_1.hpp"

#include <cmath>

using namespace std;

WarehouseSolution glouton(const WarehouseInstance& data, vector<vector<int>>& current_sequence) {
    vector<int> new_rack_capacity = newEndRackCapacity(data);
    // The products are placed in order
    vector<int> assignment(data.num_products, 0);
    int current_rack = 1;
    int current_product;
    for (int current_circuit = 0; current_circuit < (int)current_sequence.size(); current_circuit++) {
        for (int i = 0; i < (int)current_sequence[current_circuit].size(); i++) {
            current_product = current_sequence[current_circuit][i];
            while (assignment[current_product] == 0) {
                if (new_rack_capacity[current_rack] > 0) {
                    assignment[current_product] = current_rack;
                    new_rack_capacity[current_rack] -= 1;
                } else {
                    current_rack += 1;
                }
            }
        }
    }
    return WarehouseSolution(data, assignment);
}

void RL_products(const WarehouseInstance& data, WarehouseSolution& current_solution, vector<vector<int>>& current_sequence, bool& stop,
                 int& current_cost) {
    for (int f = 0; f < data.num_circuits; f++) {
        // cout << "--------------- ITERATION : " << f << endl;
        for (int k = 0; k < (int)current_sequence[f].size() - 1; k++) {
            if (current_solution.assignment[current_sequence[f][k]] != current_solution.assignment[current_sequence[f][k + 1]]) {
                swap(current_solution.assignment[current_sequence[f][k]], current_solution.assignment[current_sequence[f][k + 1]]);

                int new_cost = calculate_cost(current_solution);
                // cout << "new cost : " << new_cost << endl;

                if (new_cost < current_cost) {
                    stop = false;
                    swap(current_sequence[f][k], current_sequence[f][k + 1]);
                    current_cost = new_cost;
                } else {
                    swap(current_solution.assignment[current_sequence[f][k]], current_solution.assignment[current_sequence[f][k + 1]]);
                }
            }
        }
    }
}

void RL_circuits(const WarehouseInstance& data, WarehouseSolution& current_solution, vector<vector<int>>& current_sequence, bool& stop,
                 int& current_cost) {
    for (int f = 0; f < data.num_circuits - 1; f++) {
        swap(current_sequence[f], current_sequence[f + 1]);
        int new_cost = calculate_cost(glouton(data, current_sequence));

        if (new_cost < current_cost) {
            stop = false;
            current_solution = glouton(data, current_sequence);
        } else {
            swap(current_sequence[f], current_sequence[f + 1]);
        }
    }
}

WarehouseSolution Heur_1(const WarehouseInstance& data, int iter_max) {
    vector<vector<int>> current_sequence(data.num_circuits, vector<int>(0));
    // cout << "vec créé : " << current_sequence.size() << " " << current_sequence[0].size() << endl;
    for (int j = 0; j < data.num_products; j++) {
        current_sequence[data.product_circuit[j]].push_back(j);
    }
    WarehouseSolution current_solution = glouton(data, current_sequence);
    int current_cost = calculate_cost(current_solution);
    int iter = 0;
    bool stop = false;
    while (++iter < iter_max && !stop) {
        // cout << "iter : " << iter << endl;
        stop = true;
        RL_products(data, current_solution, current_sequence, stop, current_cost);
        // cout << "cost1 : " << current_cost << endl;
        RL_circuits(data, current_solution, current_sequence, stop, current_cost);
        // cout << "cost2 : " << current_cost << endl;
    }

    // cout << "nb d'iter : " << iter << endl << "stop : " << stop << endl << endl;

    return current_solution;
}
