#include "Heur.hpp"

#include <Utilitary.hpp>

using namespace std;

Heuristic::Heuristic(const WarehouseSolution& initial_solution) : solution(initial_solution) {
    circuit_products = buildProductInCircuit(solution.data);
    buildProductToOrders();
    buildRackUsed();
    buildCircuitIntervals();
    calculateCostAndPath();
}

void Heuristic::buildProductToOrders() {
    product_to_orders.clear();
    for (int o = 0; o < solution.data.num_orders; o++) {
        for (int product : solution.data.orders[o]) {
            product_to_orders[product].push_back(o);
        }
    }
}

void Heuristic::buildRackUsed() {
    rack_used = vector<int>(solution.data.num_racks, 0);
    for (int p = 0; p < solution.data.num_products; p++) {
        int r = solution.assignment[p];
        if (r >= 0 && r < solution.data.num_racks) {
            rack_used[r]++;
        }
    }
}

void Heuristic::buildCircuitIntervals() {
    circuit_intervals = vector<pair<int, int>>(solution.data.num_circuits, {INT_MAX, INT_MIN});
    for (int c = 0; c < solution.data.num_circuits; c++) {
        for (int p : circuit_products[c]) {
            int r = solution.assignment[p];
            if (r >= 0) {
                circuit_intervals[c].first = min(circuit_intervals[c].first, r);
                circuit_intervals[c].second = max(circuit_intervals[c].second, r);
            }
        }
        // If the circuit is empty or unassigned, we ignore (min > max)
    }
}

void Heuristic::calculateCostAndPath() {
    const auto& adj = solution.data.adjacency;
    int start = 0;
    int end = solution.data.num_racks - 1;

    solution_cost = 0;
    path.clear();
    path.resize(solution.data.num_orders);

    for (int o = 0; o < solution.data.num_orders; o++) {
        set<int> racks_set;
        for (int product : solution.data.orders[o]) {
            racks_set.insert(solution.assignment[product]);
        }

        path[o].push_back(start);
        for (int rack : racks_set) {
            path[o].push_back(rack);
        }
        path[o].push_back(end);

        for (int p = 0; p + 1 < path[o].size(); p++) {
            solution_cost += adj[path[o][p]][path[o][p + 1]];
        }
    }
}
