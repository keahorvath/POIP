#include "Heur_2.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Utilitary.hpp"
using namespace std;

static std::mt19937 rng((std::random_device())());

void Heuristic_2::improve(const int max_attempt_per_circuit, const int stagnation_threshold) {
    int compt = 0;
    for (int c = 0; c < solution.data.num_circuits; c++) {
        // cout << compt << " " << endl;
        compt++;
        if (circuit_products[c].empty() || circuit_products[c].size() < 2) continue;
        if (circuit_intervals[c].first > circuit_intervals[c].second) continue;  // Circuit not assigned, ignore

        int attempts = 0;
        int stagnant = 0;
        long long fact = factorial((int)circuit_products[c].size(), max_attempt_per_circuit);
        int max_attempts_per_circuit = (int)std::min((long long)max_attempt_per_circuit, fact);

        while (attempts < max_attempts_per_circuit && stagnant < stagnation_threshold) {
            attempts++;
            stagnant++;

            // Select two distinct random products from the circuit
            int p1, p2;
            if (!selectSwapCandidates(c, p1, p2)) continue;  // If we couldn't select valid candidates, skip this attempt

            // Calculate delta cost only on the orders affected by p1 or p2
            set<int> affected_orders = getAffectedOrders(p1, p2);
            if (affected_orders.empty()) continue;
            long long delta = calculateDelta(p1, p2, affected_orders);

            // No need to check aeration or capacities because the swap does not change the number of products per rack/aisle
            // If improvement, perform the swap and update the solution and paths of affected orders
            if (delta < 0) {
                applySwap(p1, p2, affected_orders);
                solution_cost += delta;
                stagnant = 0;
            }
        }
    }
}

bool Heuristic_2::selectSwapCandidates(int circuit_idx, int& p1, int& p2) {
    size_t idx1 = rng() % circuit_products[circuit_idx].size();
    size_t idx2 = rng() % circuit_products[circuit_idx].size();
    if (idx1 == idx2) return false;  // Make sure we have two different products
    p1 = circuit_products[circuit_idx][idx1];
    p2 = circuit_products[circuit_idx][idx2];
    return solution.assignment[p1] != solution.assignment[p2];  // No swap if same rack
}

set<int> Heuristic_2::getAffectedOrders(int p1, int p2) {
    set<int> affected_orders_set;
    for (int o : product_to_orders[p1]) affected_orders_set.insert(o);
    for (int o : product_to_orders[p2]) affected_orders_set.insert(o);
    return affected_orders_set;
}

long long Heuristic_2::calculateDelta(int p1, int p2, const set<int>& affected_orders) {
    long long delta = 0;
    int old_r1 = solution.assignment[p1];
    int old_r2 = solution.assignment[p2];
    for (int o : affected_orders) {
        // Old cost for this order
        const vector<int>& old_path = path[o];
        long long old_order_cost = 0;
        for (size_t j = 0; j + 1 < old_path.size(); j++) {
            old_order_cost += solution.data.adjacency[old_path[j]][old_path[j + 1]];
        }
        // New set of racks for this order after the swap (simulation)
        set<int> new_racks_set;
        for (int prod : solution.data.orders[o]) {
            int rr = (prod == p1) ? old_r2 : (prod == p2) ? old_r1 : solution.assignment[prod];
            new_racks_set.insert(rr);
        }
        // New path and new cost
        vector<int> new_path_vec = {0};
        for (int rr : new_racks_set) new_path_vec.push_back(rr);
        new_path_vec.push_back(solution.data.num_racks - 1);
        long long new_order_cost = 0;
        for (size_t j = 0; j + 1 < new_path_vec.size(); j++) {
            new_order_cost += solution.data.adjacency[new_path_vec[j]][new_path_vec[j + 1]];
        }
        delta += new_order_cost - old_order_cost;
    }
    return delta;
}

void Heuristic_2::applySwap(int p1, int p2, const set<int>& affected_orders) {
    // Update the assignment
    std::swap(solution.assignment[p1], solution.assignment[p2]);
    // Update the paths of affected orders
    for (int o : affected_orders) {
        set<int> updated_racks;
        for (int prod : solution.data.orders[o]) {
            updated_racks.insert(solution.assignment[prod]);
        }
        vector<int> updated_path = {0};
        for (int rr : updated_racks) updated_path.push_back(rr);
        updated_path.push_back(solution.data.num_racks - 1);
        path[o] = updated_path;
    }
}

//\\ THE RACK NUMBERS START AT 1 AND END AT 1599 SO n-2 is the number of the last rack
vector<int> buildInitialSolution(const WarehouseInstance& data, const vector<int>& frequency_circuits) {
    vector<int> assignment(data.num_products, -1);
    vector<int> rack_capacity = newRandomRackCapacity(data);  // add randomly aeration in the racks of each aisle
    vector<int> rack_content(data.num_racks - 1, 0);          // rack_content[i] = number of products currently assigned to rack i
    vector<vector<int>> product_in_circuit = buildProductInCircuit(data);
    for (int circuit : frequency_circuits) {
        for (int product : product_in_circuit[circuit]) {
            int current_aisle = 0;
            int current_rack = 0;
            bool assigned = false;
            while (current_aisle < (int)data.aisles_racks.size() && !assigned) {
                while (current_rack < (int)data.aisles_racks[current_aisle].size()) {
                    int rack = data.aisles_racks[current_aisle][current_rack];
                    if (rack_capacity[rack] > 0 && rack_content[rack] < rack_capacity[rack]) {
                        assignment[product] = rack;
                        rack_content[rack]++;
                        assigned = true;
                        break;
                    }
                    current_rack++;
                }
                if (!assigned) {
                    current_aisle++;
                    current_rack = 0;
                }
            }
            if (!assigned) {
                std::cerr << "Produit " << product << " non assigné\n";
            }
        }
    }

    // SECURITY : place all orphan products in first possible position
    for (int p = 0; p < data.num_products; p++) {
        if (assignment[p] == -1) {
            bool force_assigned = false;
            for (int i = 0; i < (int)data.aisles_racks.size() && !force_assigned; i++) {
                for (int r : data.aisles_racks[i]) {
                    if (r < (int)rack_content.size() && rack_capacity[r] > 0 && rack_content[r] < rack_capacity[r]) {
                        assignment[p] = r;
                        rack_content[r]++;
                        force_assigned = true;
                        break;
                    }
                }
            }
            if (!force_assigned) {
                cerr << "[ALERTE] Impossible de forcer le placement du produit " << p << " : Entrepôt plein !" << endl;
            } else {
                cout << "[INFO] Produit orphelin " << p << " forcé dans un rack." << endl;
            }
        }
    }
    return assignment;
}