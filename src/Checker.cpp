#include "Checker.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

using namespace std;

Checker::Checker(const WarehouseInstance data, const string& file_path) : sol(data, readSolution(file_path)) {};

vector<int> Checker::readSolution(const string& file_path) {
    ifstream file(file_path);

    if (!file.is_open()) {
        throw runtime_error("Couldn't open file : " + file_path);
    }

    int n;
    // First line : number of products
    file >> n;

    // Then, read all the rack numbers
    vector<int> assignment;
    assignment.reserve(n);

    int rack_id;
    while (file >> rack_id) {
        assignment.push_back(rack_id);
    }

    // Make sure there are as many items as announced
    if (assignment.size() != static_cast<size_t>(n)) {
        throw runtime_error("File announces " + to_string(n) + " products but only has " + to_string(assignment.size()) + " lines.");
    }

    return assignment;
}

bool Checker::checkAllProductsAssigned() {
    // Make sure each product has a valid rack
    int expected = sol.data.num_products;
    int num_racks = sol.data.rack_capacity.size();

    if (sol.assignment.size() != expected) {
        cerr << sol.assignment.size() << " given but expected " << expected << endl;
        return false;
    }
    for (int i = 0; i < sol.assignment.size(); i++) {
        int rack = sol.assignment[i];
        if (rack < 0 || rack > num_racks) {
            cerr << "Product " << i << " : rack " << rack << " invalid." << endl;
            return false;
        }
    }
    return true;
}

bool Checker::checkRackCapacity() {
    // Count number of products in each rack
    std::vector<int> counts(sol.data.rack_capacity.size(), 0);
    for (int rack_id : sol.assignment) {
        counts[rack_id]++;
    }

    // Check that we don't have more than allowed capacity
    for (int r = 0; r < counts.size(); ++r) {
        int n = counts[r];
        int cap = sol.data.rack_capacity[r];
        if (n > cap) {
            cerr << "Rack " << r << " : " << n << " products but capacity of" << cap << "." << endl;
            return false;
        }
    }
    return true;
}

bool Checker::checkAeration() {
    double rate = sol.data.aeration_rate / 100.0;

    // Count the number of product per rack
    std::vector<int> counts(sol.data.rack_capacity.size(), 0);
    for (int rack_id : sol.assignment) {
        counts[rack_id]++;
    }

    // For each aisle, calculate the total and used capacity
    for (int i = 0; i < sol.data.aisles_racks.size(); ++i) {
        const auto& aisle = sol.data.aisles_racks[i];

        int total_cap = 0;
        int used_cap = 0;
        for (int r : aisle) {
            total_cap += sol.data.rack_capacity[r];
            used_cap += counts[r];
        }

        // Calculate the minimum aeration necessary
        int min_aeration = static_cast<int>(ceil(total_cap * rate));

        // Count the current aeration
        int actual_aeration = total_cap - used_cap;

        if (actual_aeration < min_aeration) {
            cerr << "Aisle " << i << " : aeration " << actual_aeration << ", minimum required " << min_aeration << "." << std::endl;
            return false;
        }
    }
    return true;
}
bool Checker::checkCircuitContiguity() {
    // Interval of a circuit (first product of that circuit to last of the circuit)
    struct Interval {
        int min_r = -1;
        int max_r = -1;
    };

    map<int, Interval> intervals;

    // Find the correct intervals for each circruit
    for (int p = 0; p < sol.assignment.size(); ++p) {
        int c = sol.data.product_circuit[p];
        int r = sol.assignment[p];

        if (intervals.find(c) == intervals.end()) {
            intervals[c] = {r, r};
        } else {
            intervals[c].min_r = min(intervals[c].min_r, r);
            intervals[c].max_r = max(intervals[c].max_r, r);
        }
    }

    // Put circuits (with intervals) in vector so we can sort it
    struct CircuitInfo {
        int id;
        Interval range;
    };
    std::vector<CircuitInfo> sorted_circuits;
    for (auto const& [id, range] : intervals) {
        sorted_circuits.push_back({id, range});
    }

    // Sort by min bound, then sup bound
    sort(sorted_circuits.begin(), sorted_circuits.end(), [](const CircuitInfo& a, const CircuitInfo& b) {
        if (a.range.min_r != b.range.min_r) {
            return a.range.min_r < b.range.min_r;
        }
        return a.range.max_r < b.range.max_r;
    });

    // Then, check if neighbors overlap
    for (int i = 0; i + 1 < sorted_circuits.size(); ++i) {
        const auto& current = sorted_circuits[i];
        const auto& next = sorted_circuits[i + 1];

        if (current.range.max_r > next.range.min_r) {
            cerr << "Overlap : circuit " << current.id << " [" << current.range.min_r << ", " << current.range.max_r << "] "
                 << "and circuit " << next.id << " [" << next.range.min_r << ", " << next.range.max_r << "]." << ecvt_r;
            return false;
        }
    }

    return true;
}

int Checker::calculateCost() {
    int total_cost = 0;

    const auto& adj = sol.data.adjacency;
    int start_node = 0;
    int end_node = static_cast<int>(adj.size()) - 1;

    // Go through each order
    for (const auto& order : sol.data.orders) {
        // Find which racks are used
        // Using a set to make sure we only count each rack once
        set<int> unique_racks;
        for (int product_id : order) {
            unique_racks.insert(sol.assignment[product_id]);
        }

        // Building the path in order
        std::vector<int> path;
        path.push_back(start_node);
        for (int r : unique_racks) {
            path.push_back(r);
        }
        path.push_back(end_node);

        // Add the distances between each step
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            int u = path[i];
            int v = path[i + 1];
            total_cost += adj[u][v];
        }
    }

    return total_cost;
}

bool Checker::check() {
    return (checkAllProductsAssigned() && checkAeration() && checkCircuitContiguity() && checkRackCapacity());
}
