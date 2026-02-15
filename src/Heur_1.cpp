#include "Heur_1.hpp"
#include <cmath>

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

// Returns the number of free locations in each aisle (= aisle capacity * aeration percentage)
vector<int> Num_free_loc (const WarehouseInstance& data) {
    vector<int> num_free_loc(data.num_aisles, 0);
    for (int i = 0; i < data.num_aisles; i++) {
        for (int j = 0; j < (int)data.aisles_racks[i].size(); j++) {
            num_free_loc[i] += data.rack_capacity[data.aisles_racks[i][j]];
        }
        num_free_loc[i] = ceil(num_free_loc[i]*data.aeration_rate/100.);
    }
    return num_free_loc;
}

// Update the capacities of the racks according to the aeration which is arbitrairy at the end of each aisle
vector<int> New_rack_capacity (const WarehouseInstance& data) {
    vector<int> num_free_loc = Num_free_loc(data);
    vector<int> new_rack_capacity = data.rack_capacity;
    vector<int> num_racksPerAisle (data.num_aisles);
    for (int i = 0; i < data.num_aisles; i++) {
        num_racksPerAisle[i] = data.aisles_racks[i].size();
    }
    for (int i = 0; i < data.num_aisles; i++) {
        int k=1;
        while (num_free_loc[i] > data.rack_capacity[data.aisles_racks[i][num_racksPerAisle[i]-k]]) {
            new_rack_capacity[data.aisles_racks[i][num_racksPerAisle[i]-k]] = 0;
            num_free_loc[i] -= data.rack_capacity[data.aisles_racks[i][num_racksPerAisle[i]-k]];
            k++;
        }
        new_rack_capacity[data.aisles_racks[i][num_racksPerAisle[i]-k]] -= num_free_loc[i];
    }
    return new_rack_capacity;
}

WarehouseSolution Glouton (const WarehouseInstance& data, vector<vector<int>>& current_sequence){
    vector<int> new_rack_capacity = New_rack_capacity(data);
    // The products are placed in order
    vector<int> assignment (data.num_products, 0);
    int current_rack = 1; int current_product;
    for (int current_circuit = 0; current_circuit < (int)current_sequence.size(); current_circuit++) {
        for (int i = 0; i < (int)current_sequence[current_circuit].size(); i++) {
            current_product = current_sequence[current_circuit][i];
            while(assignment[current_product] == 0) {
                if(new_rack_capacity[current_rack] > 0) {
                    assignment[current_product] = current_rack;
                    new_rack_capacity[current_rack] -= 1;
                }
                else {
                    current_rack += 1;
                }
            }
        }
    }
    return WarehouseSolution(data, assignment);
}

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
        // cout << "--------------- ITERATION : " << f << endl;
        for (int k = 0; k < (int)current_sequence[f].size() - 1; k++) {

            if (current_solution.assignment[current_sequence[f][k]]
                != current_solution.assignment[current_sequence[f][k+1]]) {

                swap(current_solution.assignment[current_sequence[f][k]],
                    current_solution.assignment[current_sequence[f][k+1]]);

                int new_cost = calculate_cost(current_solution);
                // cout << "new cost : " << new_cost << endl;

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
    // cout << "vec créé : " << current_sequence.size() << " " << current_sequence[0].size() << endl;
    for (int j = 0; j < data.num_products; j++) {
        current_sequence[data.product_circuit[j]].push_back(j);
    }
    WarehouseSolution current_solution = Glouton (data, current_sequence);
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
