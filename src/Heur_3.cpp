#include "Heur_3.hpp"

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

using namespace std;

static mt19937 rng((random_device())());
vector<int> Heuristic_3::buildInitialSolution(const vector<int>& frequency_circuits, vector<vector<int>> freq_products,
                                              unordered_map<int, vector<int>> product_pairs, vector<vector<int>> never_used) {
    // --- 1) INIT ---
    solution.assignment.assign(solution.data.num_products, -1);
    rack_content.assign(solution.data.num_racks, vector<int>());
    vector<bool> used_products(solution.data.num_products, false);

    // --- 2) AERATION ---
    vector<int> aisle_aeration = calculateAisleAeration();
    placeInitialAeration(aisle_aeration);

    // --- 3) FILL BY CIRCUIT ---
    for (int circuit : frequency_circuits) {
        reverse(freq_products[circuit].begin(), freq_products[circuit].end());
        reverse(never_used[circuit].begin(), never_used[circuit].end());
        fillWarehouseForCircuit(circuit, freq_products[circuit], never_used[circuit], product_pairs, used_products);
    }
    return solution.assignment;
}

vector<int> Heuristic_3::calculateAisleAeration() {
    vector<int> aisle_aeration(solution.data.num_aisles, 0);

    for (int i = 0; i < solution.data.num_aisles; i++) {
        int aisle_cap = 0;
        for (int rack : solution.data.aisles_racks[i]) {
            aisle_cap += solution.data.rack_capacity[rack];
        }
        aisle_aeration[i] = (int)ceil(aisle_cap * solution.data.aeration_rate / 100.0);
    }
    return aisle_aeration;
}

void Heuristic_3::placeInitialAeration(vector<int>& aisle_aeration) {
    for (int i = 0; i < solution.data.num_aisles; i++) {
        // Go through aisles in order
        vector<int> racks = solution.data.aisles_racks[i];
        sort(racks.begin(), racks.end());

        for (int rack : racks) {
            if (rack % 2 == 1) {  // odd rack = left
                while ((int)rack_content[rack].size() < solution.data.rack_capacity[rack] && aisle_aeration[i] > 0) {
                    rack_content[rack].push_back(-1);
                    aisle_aeration[i]--;
                }
            }
        }
    }
}

void Heuristic_3::fillWarehouseForCircuit(int circuit, vector<int>& freqs, vector<int>& nus, unordered_map<int, vector<int>>& pairs,
                                          vector<bool>& used) {
    for (int aisle = 0; aisle < solution.data.num_aisles; aisle++) {
        vector<int> racks = solution.data.aisles_racks[aisle];
        sort(racks.begin(), racks.end());
        for (int rack : racks) {
            while (rackHasSpace(rack)) {
                bool added = false;
                if (rack % 2 == 0) {
                    added = fillEvenRack(freqs, nus, pairs, rack, used);
                } else {
                    added = fillOddRack(freqs, nus, pairs, rack, used);
                }
                if (!added) break;
            }
            if (isCircuitEmpty(freqs, nus, used)) return;
        }
    }
}

bool Heuristic_3::fillEvenRack(vector<int>& freqs, vector<int>& nus, unordered_map<int, vector<int>>& pairs, int rack, vector<bool>& used) {
    int p_main = popNextValid(freqs, used);
    if (p_main != -1) {
        placeProduct(p_main, rack, used);
        // Partners
        if (pairs.count(p_main)) {
            for (int p_partner : pairs[p_main]) {
                if (!rackHasSpace(rack)) break;
                if (!used[p_partner]) placeProduct(p_partner, rack, used);
            }
        }
        // Complete with NU
        while (rackHasSpace(rack)) {
            int p_nu = popNextValid(nus, used);
            if (p_nu == -1) break;
            placeProduct(p_nu, rack, used);
        }
        return true;
    } else {
        int p_nu = popNextValid(nus, used);
        if (p_nu != -1) {
            placeProduct(p_nu, rack, used);
            return true;
        }
    }
    return false;
}

bool Heuristic_3::fillOddRack(vector<int>& freqs, vector<int>& nus, unordered_map<int, vector<int>>& pairs, int rack, vector<bool>& used) {
    int p_nu = popNextValid(nus, used);
    if (p_nu != -1) {
        placeProduct(p_nu, rack, used);
        // Complete with NU
        while (rackHasSpace(rack)) {
            int p2 = popNextValid(nus, used);
            if (p2 == -1) break;
            placeProduct(p2, rack, used);
        }
        // If still space, add frequent + partners
        while (rackHasSpace(rack)) {
            int p_f = popNextValid(freqs, used);
            if (p_f == -1) break;
            placeProduct(p_f, rack, used);
            if (pairs.count(p_f)) {
                for (int p_p : pairs[p_f]) {
                    if (!rackHasSpace(rack)) break;
                    if (!used[p_p]) placeProduct(p_p, rack, used);
                }
            }
        }
        return true;
    } else {
        int p_f = popNextValid(freqs, used);
        if (p_f != -1) {
            placeProduct(p_f, rack, used);
            if (pairs.count(p_f)) {
                for (int p_p : pairs[p_f]) {
                    if (!rackHasSpace(rack)) break;
                    if (!used[p_p]) placeProduct(p_p, rack, used);
                }
            }
            return true;
        }
    }
    return false;
}
int Heuristic_3::popNextValid(vector<int>& pile, vector<bool>& used) {
    while (!pile.empty()) {
        int p = pile.back();
        pile.pop_back();
        if (!used[p]) return p;
    }
    return -1;
}

void Heuristic_3::placeProduct(int p, int rack, vector<bool>& used) {
    rack_content[rack].push_back(p);
    solution.assignment[p] = rack;
    used[p] = true;
}

bool Heuristic_3::isCircuitEmpty(const vector<int>& freqs, const vector<int>& nus, const vector<bool>& used) {
    for (int p : freqs)
        if (!used[p]) return false;
    for (int p : nus)
        if (!used[p]) return false;
    return true;
}

bool Heuristic_3::rackHasSpace(int rack) {
    return (int)rack_content[rack].size() < solution.data.rack_capacity[rack];
}

void Heuristic_3::improve(const int max_attempt_per_circuit, const int stagnation_threshold, bool allow_aeration_swaps) {
    buildProductToOrders();
    buildRackUsed();
    buildCircuitIntervals();
    calculateCostAndPath();
    if (rack_to_aisle.empty() || (int)rack_to_aisle.size() != solution.data.num_racks) {
        buildRackToAisle();
    }
    if (best_cost == LLONG_MAX) {
        best_cost = solution_cost;
    }
    std::uniform_real_distribution<double> uni01(0.0, 1.0);
    int compt = 0;
    // For each circuit independently
    for (int c = 0; c < solution.data.num_circuits; c++) {
        if (circuit_products[c].size() < 2) continue;
        int min_r = circuit_intervals[c].first;
        int max_r = circuit_intervals[c].second;
        if (min_r < 0 || max_r < 0 || min_r > max_r) continue;
        // Tabu tenure proportional to the size of the interval
        int interval_size = max_r - min_r + 1;
        tabu_tenure = std::max(4, std::min(200, interval_size / 2));
        tabu_list.clear();
        int attempts, stagnant = 0;
        long long fact = factorial((int)circuit_products[c].size(), max_attempt_per_circuit);
        int max_attempts_per_circuit = (int)std::min((long long)max_attempt_per_circuit, fact);
        cout << " " << compt << endl;
        compt++;

        while (attempts < max_attempts_per_circuit && stagnant < stagnation_threshold) {
            attempts++;
            stagnant++;
            int p1 = -1, p2 = -1, r1 = -1, r2 = -1;
            bool is_swap_with_empty = false;
            // Call generateMove to get a candidate move (swap or insertion) for this circuit, ensuring it respects the interval and aeration
            // constraints
            if (!generateMove(c, min_r, max_r, interval_size, allow_aeration_swaps, uni01, p1, p2, r1, r2, is_swap_with_empty)) {
                continue;
            }
            bool tabu = isTabu(r1, r2);  // Tabu check on racks

            // Calculate delta on affected orders only
            std::set<int> affected_orders_set = getAffectedOrders(p1, p2, is_swap_with_empty);
            if (affected_orders_set.empty()) continue;
            long long delta = calculateDelta(p1, p2, r1, r2, is_swap_with_empty, affected_orders_set);

            // Aspiration : allow tabu move if it improves the best known solution
            bool aspiration = (solution_cost + delta < best_cost);
            if (tabu && !aspiration) {
                continue;
            }

            // Acceptance criteria
            bool accept = false;
            if (delta < 0) {
                accept = true;
            } else {
                // Sometimes accept non-improving moves if stagnation is high, to escape local minima
                if (stagnant > stagnation_threshold / 3) {
                    if (uni01(rng) < proba_accept_non_improving) {
                        accept = true;
                    }
                }
            }
            if (!accept) continue;

            // Update
            applyAndCommitMove(p1, p2, r1, r2, is_swap_with_empty, affected_orders_set, delta);
            stagnant = 0;
        }
    }
}

bool Heuristic_3::generateMove(int c, int min_r, int max_r, int interval_size, bool allow_aeration_swaps,
                               std::uniform_real_distribution<double>& uni01, int& p1, int& p2, int& r1, int& r2, bool& is_swap_with_empty) {
    // Pick move type
    bool try_aeration = allow_aeration_swaps && (uni01(rng) < proba_swap_aeration);

    // Build valid move (without exiting circuit)
    if (!try_aeration) {
        // MOVE : product <-> product (same circuit)
        size_t idx1 = rng() % circuit_products[c].size();
        size_t idx2 = rng() % circuit_products[c].size();
        if (idx1 == idx2) return false;

        p1 = circuit_products[c][idx1];
        p2 = circuit_products[c][idx2];
        r1 = solution.assignment[p1];
        r2 = solution.assignment[p2];

        if (r1 < 0 || r2 < 0) return false;
        if (r1 >= solution.data.num_racks || r2 >= solution.data.num_racks) return false;
        if (r1 == r2) return false;

        // security : both racks must be within the interval
        if (r1 < min_r || r1 > max_r) return false;
        if (r2 < min_r || r2 > max_r) return false;
        is_swap_with_empty = false;
    } else {
        // MOVE : product <-> empty slot (aeration)
        size_t idx1 = rng() % circuit_products[c].size();
        p1 = circuit_products[c][idx1];
        r1 = solution.assignment[p1];
        if (r1 < 0 || r1 >= solution.data.num_racks) return false;
        if (r1 < min_r || r1 > max_r) return false;

        // Find a rack with an empty slot in the circuit interval
        // try a few random racks in the interval to find one with an empty slot
        bool found = false;
        for (int tries = 0; tries < 30; tries++) {
            int rr = min_r + (rng() % interval_size);
            int empty_pos = findEmptySlotInRack(rr);
            if (empty_pos != -1) {
                r2 = rr;
                found = true;
                break;
            }
        }
        if (!found) return false;
        if (r2 == r1) return false;
        // security : stay within circuit interval
        if (r2 < min_r || r2 > max_r) return false;
        // security : stay in the same aisle (otherwise you break aeration)
        if (rack_to_aisle[r1] != rack_to_aisle[r2]) return false;

        is_swap_with_empty = true;
    }
    return true;
}

long long Heuristic_3::calculateDelta(int p1, int p2, int r1, int r2, bool is_swap_with_empty, const std::set<int>& affected_orders_set) {
    long long delta = 0;
    for (int o : affected_orders_set) {
        // old cost
        const vector<int>& old_path = path[o];
        long long old_cost = 0;
        for (size_t j = 0; j + 1 < old_path.size(); j++) {
            old_cost += solution.data.adjacency[old_path[j]][old_path[j + 1]];
        }
        // racks after move
        set<int> new_racks_set;
        for (int prod : solution.data.orders[o]) {
            int rr = solution.assignment[prod];
            if (prod == p1) {
                rr = r2;
            } else if (!is_swap_with_empty && prod == p2) {
                rr = r1;
            }
            new_racks_set.insert(rr);
        }
        // new path
        vector<int> new_path;
        new_path.push_back(0);
        for (int rr : new_racks_set) new_path.push_back(rr);
        new_path.push_back(solution.data.num_racks - 1);

        long long new_cost = 0;
        for (size_t j = 0; j + 1 < new_path.size(); j++) {
            new_cost += solution.data.adjacency[new_path[j]][new_path[j + 1]];
        }
        delta += (new_cost - old_cost);
    }
    return delta;
}

void Heuristic_3::applyAndCommitMove(int p1, int p2, int r1, int r2, bool is_swap_with_empty, const std::set<int>& affected_orders_set,
                                     long long delta) {
    // Apply move (keep rack_content coherent)
    if (!is_swap_with_empty) {
        // swap product-product
        // update assignment
        solution.assignment[p1] = r2;
        solution.assignment[p2] = r1;
        // update rack_content
        int pos1 = findInRack(r1, p1);
        int pos2 = findInRack(r2, p2);
        if (pos1 != -1 && pos2 != -1) {
            swap(rack_content[r1][pos1], rack_content[r2][pos2]);
        }
    } else {
        // swap product <-> empty slot
        solution.assignment[p1] = r2;
        int pos_prod = findInRack(r1, p1);
        int pos_empty = findEmptySlotInRack(r2);
        if (pos_prod != -1 && pos_empty != -1) {
            rack_content[r1][pos_prod] = -1;
            rack_content[r2][pos_empty] = p1;
        }
    }
    // update cost and paths for affected orders
    solution_cost += delta;
    for (int o : affected_orders_set) {
        set<int> updated_racks;
        for (int prod : solution.data.orders[o]) {
            updated_racks.insert(solution.assignment[prod]);
        }
        vector<int> updated_path;
        updated_path.push_back(0);
        for (int rr : updated_racks) updated_path.push_back(rr);
        updated_path.push_back(solution.data.num_racks - 1);
        path[o] = updated_path;
    }
    pushTabu(r1, r2);
    if (solution_cost < best_cost) {
        best_cost = solution_cost;
    }
}

set<int> Heuristic_3::getAffectedOrders(int p1, int p2, bool is_swap_with_empty) {
    set<int> affected;
    for (int o : product_to_orders[p1]) affected.insert(o);
    if (!is_swap_with_empty && p2 != -1) {
        for (int o : product_to_orders[p2]) affected.insert(o);
    }
    return affected;
}

void Heuristic_3::buildRackToAisle() {
    rack_to_aisle.assign(solution.data.num_racks, -1);
    for (int a = 0; a < solution.data.num_aisles; a++) {
        for (int r : solution.data.aisles_racks[a]) {
            rack_to_aisle[r] = a;
        }
    }
}

int Heuristic_3::findInRack(int rack, int product) const {
    for (int i = 0; i < (int)rack_content[rack].size(); i++) {
        if (rack_content[rack][i] == product) return i;
    }
    return -1;
}

bool Heuristic_3::isTabu(int r1, int r2) const {
    if (r1 > r2) std::swap(r1, r2);
    for (const auto& mv : tabu_list) {
        if (mv.r1 == r1 && mv.r2 == r2) return true;
    }
    return false;
}

int Heuristic_3::findEmptySlotInRack(int rack) const {
    for (int i = 0; i < (int)rack_content[rack].size(); i++) {
        if (rack_content[rack][i] == -1) return i;
    }
    return -1;
}

void Heuristic_3::pushTabu(int r1, int r2) {
    if (r1 > r2) std::swap(r1, r2);
    tabu_list.push_back({r1, r2});
    if ((int)tabu_list.size() > tabu_tenure) tabu_list.pop_front();
}