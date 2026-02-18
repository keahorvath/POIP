#include "Heur_3.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <climits>
#include <random>
#include <set>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;

static mt19937 rng((random_device())());

Heuristic_3::Heuristic_3() {} // default constructor

Heuristic_3::Heuristic_3(const WarehouseSolution& initial_solution) {
    solution.data = initial_solution.data;
    solution.assignment = initial_solution.assignment;
    circuit_products = build_product_in_circuit(initial_solution.data);
    build_product_to_orders();
    build_rack_used();
    build_circuit_intervals();
    calcul_cost_and_path();
}

vector<int> Heuristic_3::initial_solution3(
    const vector<int>& frequency_circuits,
    vector<vector<int>> freq_products,
    unordered_map<int, vector<int>> product_pairs,
    vector<vector<int>> never_used
) {
    // --- 1) INIT ---
    frequency_circuits = build_circuit_order_by_concordance_fast_weighted(frequency_circuits, freq_products, freq_count);
    solution.assignment.assign(solution.data.num_products, -1);
    rack_content.assign(solution.data.num_racks, vector<int>());

    vector<bool> used_products(solution.data.num_products, false);

    // --- 2) AERATION (on la place au début dans les racks impairs) ---
    vector<int> aisle_aeration(solution.data.num_aisles, 0);

    for (int i = 0; i < solution.data.num_aisles; i++) {
        int aisle_cap = 0;
        for (int rack : solution.data.aisles_racks[i]) {
            aisle_cap += solution.data.rack_capacity[rack];
        }
        aisle_aeration[i] = (int)ceil(aisle_cap * solution.data.aeration_rate / 100.0);
    }

    for (int i = 0; i < solution.data.num_aisles; i++) {
        // on parcourt les racks de l'allée dans l'ordre
        vector<int> racks = solution.data.aisles_racks[i];
        sort(racks.begin(), racks.end());

        for (int rack : racks) {
            if (rack % 2 == 1) { // rack impair = gauche
                while ((int)rack_content[rack].size() < solution.data.rack_capacity[rack] &&
                       aisle_aeration[i] > 0) {
                    rack_content[rack].push_back(-1);
                    aisle_aeration[i]--;
                }
            }
        }
    }

    // --- Petites lambdas utiles ---
    auto place_product_in_rack = [&](int p, int rack) {
        rack_content[rack].push_back(p);
        solution.assignment[p] = rack;
        used_products[p] = true;
    };

    auto rack_has_space = [&](int rack) -> bool {
        return (int)rack_content[rack].size() < solution.data.rack_capacity[rack];
    };

    // --- 3) Pour chaque circuit, on va remplir des racks consécutifs ---
    // On parcourt les circuits dans l'ordre donné.
    for (int circuit : frequency_circuits) {

        // On va transformer les listes en "piles" (pop_back rapide)
        // -> donc on inverse pour garder l'ordre initial si besoin.
        reverse(freq_products[circuit].begin(), freq_products[circuit].end());
        reverse(never_used[circuit].begin(), never_used[circuit].end());

        // --- On parcourt tout l'entrepôt dans un ordre monotone ---
        for (int aisle = 0; aisle < solution.data.num_aisles; aisle++) {

            vector<int> racks = solution.data.aisles_racks[aisle];
            sort(racks.begin(), racks.end());

            for (int rack : racks) {

                // Tant qu'on peut remplir ce rack avec des produits de CE circuit
                while (rack_has_space(rack)) {

                    bool is_even = (rack % 2 == 0);

                    // --- Helpers pour récupérer un produit fréquent / never_used non utilisé ---
                    auto pop_next_freq = [&]() -> int {
                        while (!freq_products[circuit].empty()) {
                            int p = freq_products[circuit].back();
                            freq_products[circuit].pop_back();
                            if (!used_products[p]) return p;
                        }
                        return -1;
                    };

                    auto pop_next_never = [&]() -> int {
                        while (!never_used[circuit].empty()) {
                            int p = never_used[circuit].back();
                            never_used[circuit].pop_back();
                            if (!used_products[p]) return p;
                        }
                        return -1;
                    };

                    // ---------------------------------------------------------
                    // RACK PAIR : priorité au fréquent
                    // ---------------------------------------------------------
                    if (is_even) {
                        int p_main = -1;

                        // 1) essayer fréquent
                        if (!freq_products[circuit].empty()) {
                            p_main = pop_next_freq();
                        }

                        if (p_main != -1) {
                            // placer le produit fréquent principal
                            place_product_in_rack(p_main, rack);

                            // placer ses partenaires
                            if (product_pairs.count(p_main)) {
                                for (int p_partner : product_pairs[p_main]) {
                                    if (!rack_has_space(rack)) break;
                                    if (!used_products[p_partner]) {
                                        place_product_in_rack(p_partner, rack);
                                    }
                                }
                            }

                            // compléter avec never_used si on peut
                            while (rack_has_space(rack)) {
                                int p_nu = pop_next_never();
                                if (p_nu == -1) break;
                                place_product_in_rack(p_nu, rack);
                            }

                        } else {
                            // plus de fréquent -> on met never_used
                            int p_nu = pop_next_never();
                            if (p_nu == -1) break; // rien à mettre dans ce rack
                            place_product_in_rack(p_nu, rack);
                        }
                    }

                    // ---------------------------------------------------------
                    // RACK IMPAIR : priorité au never_used
                    // ---------------------------------------------------------
                    else {
                        int p_nu = -1;

                        // 1) essayer never_used
                        if (!never_used[circuit].empty()) {
                            p_nu = pop_next_never();
                        }

                        if (p_nu != -1) {
                            place_product_in_rack(p_nu, rack);

                            // compléter avec never_used
                            while (rack_has_space(rack)) {
                                int p2 = pop_next_never();
                                if (p2 == -1) break;
                                place_product_in_rack(p2, rack);
                            }

                            // si on a encore de la place -> mettre fréquent + partenaires
                            while (rack_has_space(rack)) {
                                int p_main = pop_next_freq();
                                if (p_main == -1) break;

                                place_product_in_rack(p_main, rack);

                                if (product_pairs.count(p_main)) {
                                    for (int p_partner : product_pairs[p_main]) {
                                        if (!rack_has_space(rack)) break;
                                        if (!used_products[p_partner]) {
                                            place_product_in_rack(p_partner, rack);
                                        }
                                    }
                                }
                            }

                        } else {
                            // plus de never_used -> on met fréquent
                            int p_main = pop_next_freq();
                            if (p_main == -1) break;
                            place_product_in_rack(p_main, rack);

                            if (product_pairs.count(p_main)) {
                                for (int p_partner : product_pairs[p_main]) {
                                    if (!rack_has_space(rack)) break;
                                    if (!used_products[p_partner]) {
                                        place_product_in_rack(p_partner, rack);
                                    }
                                }
                            }
                        }
                    }
                }

                // Condition d'arrêt :
                // si ce circuit n'a plus aucun produit à placer, on sort.
                bool freq_left = false;
                for (int p : freq_products[circuit]) {
                    if (!used_products[p]) { freq_left = true; break; }
                }
                bool nu_left = false;
                for (int p : never_used[circuit]) {
                    if (!used_products[p]) { nu_left = true; break; }
                }

                if (!freq_left && !nu_left) {
                    break;
                }
            }

            // Re-check après l'allée
            bool freq_left = false;
            for (int p : freq_products[circuit]) {
                if (!used_products[p]) { freq_left = true; break; }
            }
            bool nu_left = false;
            for (int p : never_used[circuit]) {
                if (!used_products[p]) { nu_left = true; break; }
            }

            if (!freq_left && !nu_left) {
                break;
            }
        }
    }

    return solution.assignment;
}

void Heuristic_3::build_rack_to_aisle() {
    rack_to_aisle.assign(solution.data.num_racks, -1);

    for (int a = 0; a < solution.data.num_aisles; a++) {
        for (int r : solution.data.aisles_racks[a]) {
            rack_to_aisle[r] = a;
        }
    }
}

int Heuristic_3::find_in_rack(int rack, int product) const {
    for (int i = 0; i < (int)rack_content[rack].size(); i++) {
        if (rack_content[rack][i] == product) return i;
    }
    return -1;
}


bool Heuristic_3::is_tabu(int r1, int r2) const {
    if (r1 > r2) std::swap(r1, r2);
    for (const auto& mv : tabu_list) {
        if (mv.r1 == r1 && mv.r2 == r2) return true;
    }
    return false;
}

int Heuristic_3::find_empty_slot_in_rack(int rack) const {
    for (int i = 0; i < (int)rack_content[rack].size(); i++) {
        if (rack_content[rack][i] == -1) return i;
    }
    return -1;
}


void Heuristic_3::push_tabu(int r1, int r2) {
    if (r1 > r2) std::swap(r1, r2);

    tabu_list.push_back({r1, r2});
    if ((int)tabu_list.size() > tabu_tenure) {
        tabu_list.pop_front();
    }
}


void Heuristic_3::improve3(const int max_attempt_per_circuit,const int stagnation_threshold, bool allow_aeration_swaps) {
    build_product_to_orders();
    build_rack_used();
    build_circuit_intervals();
    calcul_cost_and_path();

    if (rack_to_aisle.empty() || (int)rack_to_aisle.size() != solution.data.num_racks) {
        build_rack_to_aisle();
    }

    if (best_cost == LLONG_MAX) {
        best_cost = solution_cost;
    }

    std::uniform_real_distribution<double> uni01(0.0, 1.0);
    int compt = 0;

    // Pour chaque circuit indépendamment
    for (int c = 0; c < solution.data.num_circuits; c++) {

        if (circuit_products[c].size() < 2) continue;

        int min_r = circuit_intervals[c].first;
        int max_r = circuit_intervals[c].second;
        if (min_r < 0 || max_r < 0 || min_r > max_r) continue;

        // Tabu tenure proportionnel à la taille de l’intervalle
        int interval_size = max_r - min_r + 1;
        tabu_tenure = std::max(4, std::min(200, interval_size / 2));

        tabu_list.clear();

        int attempts = 0;
        int stagnant = 0;

        long long fact = factoriel((int)circuit_products[c].size(), max_attempt_per_circuit);

        int max_attempts_per_circuit = (int)std::min((long long)max_attempt_per_circuit, fact);
        cout << " " << compt << endl;
        compt++;


        while (attempts < max_attempts_per_circuit && stagnant < stagnation_threshold) {
            attempts++;
            stagnant++;

            // ------------------------------------------------------------
            // 1) Tirage du type de move
            // ------------------------------------------------------------
            bool try_aeration = allow_aeration_swaps && (uni01(rng) < proba_swap_aeration);

            // ------------------------------------------------------------
            // 2) Construire un move valide (sans sortir du circuit)
            // ------------------------------------------------------------
            int p1 = -1, p2 = -1;
            int r1 = -1, r2 = -1;

            bool is_swap_with_empty = false;

            // ============ Move produit-produit (même circuit) ============
            if (!try_aeration) {
                size_t idx1 = rng() % circuit_products[c].size();
                size_t idx2 = rng() % circuit_products[c].size();
                if (idx1 == idx2) continue;

                p1 = circuit_products[c][idx1];
                p2 = circuit_products[c][idx2];

                r1 = solution.assignment[p1];
                r2 = solution.assignment[p2];

                if (r1 < 0 || r2 < 0) continue;
                if (r1 >= solution.data.num_racks || r2 >= solution.data.num_racks) continue;


                if (r1 == r2) continue;

                // sécurité : les 2 racks doivent être dans l’intervalle
                if (r1 < min_r || r1 > max_r) continue;
                if (r2 < min_r || r2 > max_r) continue;
            }

            // ============ Move produit-aération ============
            else {
                // choisir un produit du circuit
                size_t idx1 = rng() % circuit_products[c].size();
                p1 = circuit_products[c][idx1];
                r1 = solution.assignment[p1];

                if (r1 < 0 || r1 >= solution.data.num_racks) continue;
    
                if (r1 < min_r || r1 > max_r) continue;

                // trouver un rack dans l'intervalle qui a un slot vide
                // on essaye quelques racks au hasard
                bool found = false;
                for (int tries = 0; tries < 30; tries++) {
                    int rr = min_r + (rng() % interval_size);
                    int empty_pos = find_empty_slot_in_rack(rr);
                    if (empty_pos != -1) {
                        r2 = rr;
                        found = true;
                        break;
                    }
                }
                if (!found) continue;

                if (r2 == r1) continue;

                // sécurité : rester dans l'intervalle du circuit
                if (r2 < min_r || r2 > max_r) continue;

                // sécurité : rester dans la même allée (sinon tu détruis l'aération)
                if (rack_to_aisle[r1] != rack_to_aisle[r2]) continue;

                // swap produit <-> slot vide
                is_swap_with_empty = true;
            }

            // ------------------------------------------------------------
            // 3) Tabu check (sur les racks)
            // ------------------------------------------------------------
            bool tabu = is_tabu(r1, r2);

            // ------------------------------------------------------------
            // 4) Calcul delta sur commandes affectées
            // ------------------------------------------------------------
            std::set<int> affected_orders_set;

            for (int o : product_to_orders[p1]) affected_orders_set.insert(o);
            if (!is_swap_with_empty) {
                for (int o : product_to_orders[p2]) affected_orders_set.insert(o);
            }
            for (int o : affected_orders_set) {
                if (o < 0 || o >= (int)path.size()) {
                    std::cerr << "[BUG] order index o=" << o 
                            << " hors limites path.size()=" << path.size() << std::endl;
                    return;
                }
            }
            if (affected_orders_set.empty()) continue;

            long long delta = 0;

            for (int o : affected_orders_set) {

                // coût ancien
                const std::vector<int>& old_path = path[o];
                long long old_cost = 0;
                for (size_t j = 0; j + 1 < old_path.size(); j++) {
                    old_cost += solution.data.adjacency[old_path[j]][old_path[j + 1]];
                }

                // racks après move
                std::set<int> new_racks_set;

                for (int prod : solution.data.orders[o]) {
                    int rr = solution.assignment[prod];

                    if (prod == p1) {
                        rr = r2;
                    } else if (!is_swap_with_empty && prod == p2) {
                        rr = r1;
                    }

                    new_racks_set.insert(rr);
                }

                // nouveau path
                std::vector<int> new_path;
                new_path.push_back(0);
                for (int rr : new_racks_set) new_path.push_back(rr);
                new_path.push_back(solution.data.num_racks - 1);

                long long new_cost = 0;
                for (size_t j = 0; j + 1 < new_path.size(); j++) {
                    new_cost += solution.data.adjacency[new_path[j]][new_path[j + 1]];
                }

                delta += (new_cost - old_cost);
            }

            // ------------------------------------------------------------
            // 5) Aspiration : autoriser tabou si améliore best_cost
            // ------------------------------------------------------------
            bool aspiration = (solution_cost + delta < best_cost);

            if (tabu && !aspiration) {
                continue;
            }

            // ------------------------------------------------------------
            // 6) Règle d'acceptation
            // ------------------------------------------------------------
            bool accept = false;

            if (delta < 0) {
                accept = true;
            } else {
                // accepter parfois des non-améliorants si stagnation grande
                if (stagnant > stagnation_threshold / 3) {
                    if (uni01(rng) < proba_accept_non_improving) {
                        accept = true;
                    }
                }
            }

            if (!accept) continue;

            // ------------------------------------------------------------
            // 7) Appliquer le move (en gardant rack_content cohérent)
            // ------------------------------------------------------------

            if (!is_swap_with_empty) {
                // swap produit-produit
                // update assignment
                solution.assignment[p1] = r2;
                solution.assignment[p2] = r1;

                // update rack_content
                int pos1 = find_in_rack(r1, p1);
                int pos2 = find_in_rack(r2, p2);
                if (pos1 == -1 || pos2 == -1) {
                    // incohérence -> on annule en mode safe
                    solution.assignment[p1] = r1;
                    solution.assignment[p2] = r2;
                    continue;
                }

                std::swap(rack_content[r1][pos1], rack_content[r2][pos2]);
            } else {
                // swap produit <-> slot vide
                solution.assignment[p1] = r2;

                int pos_prod = find_in_rack(r1, p1);
                int pos_empty = find_empty_slot_in_rack(r2);

                if (pos_prod == -1 || pos_empty == -1) {
                    solution.assignment[p1] = r1;
                    continue;
                }

                rack_content[r1][pos_prod] = -1;
                rack_content[r2][pos_empty] = p1;
            }

            // ------------------------------------------------------------
            // 8) Update cost + paths
            // ------------------------------------------------------------
            solution_cost += delta;

            for (int o : affected_orders_set) {
                std::set<int> updated_racks;
                for (int prod : solution.data.orders[o]) {
                    updated_racks.insert(solution.assignment[prod]);
                }

                std::vector<int> updated_path;
                updated_path.push_back(0);
                for (int rr : updated_racks) updated_path.push_back(rr);
                updated_path.push_back(solution.data.num_racks - 1);

                path[o] = updated_path;
            }

            // ------------------------------------------------------------
            // 9) Tabu push
            // ------------------------------------------------------------
            push_tabu(r1, r2);

            // ------------------------------------------------------------
            // 10) Best / stagnation
            // ------------------------------------------------------------
            if (solution_cost < best_cost) {
                best_cost = solution_cost;
            }

            stagnant = 0;
        }
    }
}



/*
vector<int> Heuristic_3::initial_solution3(const vector<int>& frequency_circuits, vector<vector<int>> freq_products, unordered_map<int, vector<int>> product_pairs, vector<vector<int>> never_used) {
    //Génère une solution initiale en assignant les produits des circuits les plus fréquents aux racks les plus proches de la sortie. 
    //L'aération des positionné en bas à gauche de chaque allée.
    //Les produits sont regroupées par pairs de produits les plus frequemment utilisés.

    // --- 1. INITIALISATION ---
    solution.assignment.assign(solution.data.num_products, -1);
    rack_content.assign(solution.data.num_racks, vector<int>());
    vector<bool> used_products(solution.data.num_products, false);
    vector<int> aisle_aeration(solution.data.num_aisles, 0);
    
    // Calcul du quota d'aération par allée
    for (int i = 0; i < solution.data.num_aisles; i++) {
        int aisle_cap = 0;
        for (int rack : solution.data.aisles_racks[i]) {
            aisle_cap += solution.data.rack_capacity[rack];
        }
        aisle_aeration[i] = ceil(aisle_cap * solution.data.aeration_rate / 100.0);
    }

    // --- 2. PLACEMENT DE L'AÉRATION (Une seule fois au début) ---
    for (int i = 0; i < solution.data.num_aisles; i++) {
        for (int rack : solution.data.aisles_racks[i]) {
            if (rack % 2 == 1) { // Racks de gauche
                while ((int)rack_content[rack].size() < solution.data.rack_capacity[rack] && aisle_aeration[i] > 0) {
                    rack_content[rack].push_back(-1); 
                    aisle_aeration[i]--;
                }
            }
        }
    }

    // Curseurs persistants pour assurer le remplissage continu et le mélange des familles
    int cur_aisle_nu = 0;   // Pour le placement "gauche" des never_used
    int cur_rack_nu = 0;
    int cur_aisle_freq = 0; // Pour les produits actifs et le surplus
    int cur_rack_freq = 0;

    // --- 3. BOUCLE PAR CIRCUIT ---
    for (int circuit : frequency_circuits) {
        
        // --- PHASE A : Tentative initiale pour never_used (Racks de gauche uniquement) ---
        vector<int> leftover_nu; 
        while (!never_used[circuit].empty()) {
            // Si on a parcouru tout l'entrepôt pour les racks de gauche, le reste va en leftover
            if (cur_aisle_nu >= solution.data.num_aisles) {
                while(!never_used[circuit].empty()) {
                    leftover_nu.push_back(never_used[circuit].back());
                    never_used[circuit].pop_back();
                }
                break;
            }

            int rack = solution.data.aisles_racks[cur_aisle_nu][cur_rack_nu];
            if (rack % 2 == 1 && (int)rack_content[rack].size() < solution.data.rack_capacity[rack]) {
                int p = never_used[circuit].back();
                rack_content[rack].push_back(p);
                solution.assignment[p] = rack;
                used_products[p] = true;
                never_used[circuit].pop_back();
            } else {
                cur_rack_nu++;
                if (cur_rack_nu >= (int)solution.data.aisles_racks[cur_aisle_nu].size()) {
                    cur_rack_nu = 0;
                    cur_aisle_nu++;
                }
            }
        }

        // --- PHASE B : Placement des produits actifs (Freq + Pairs) ---
        // On utilise cur_aisle_freq pour boucher tous les trous laissés précédemment
        for (int p_main : freq_products[circuit]) {
            if (used_products[p_main]) continue;

            bool assigned = false;
            while (!assigned && cur_aisle_freq < solution.data.num_aisles) {
                int rack = solution.data.aisles_racks[cur_aisle_freq][cur_rack_freq];

                if ((int)rack_content[rack].size() < solution.data.rack_capacity[rack]) {
                    rack_content[rack].push_back(p_main);
                    solution.assignment[p_main] = rack;
                    used_products[p_main] = true;
                    assigned = true;

                    if (product_pairs.count(p_main)) {
                        for (int p_partner : product_pairs[p_main]) {
                            if ((int)rack_content[rack].size() >= solution.data.rack_capacity[rack]) break;
                            if (!used_products[p_partner]) {
                                rack_content[rack].push_back(p_partner);
                                solution.assignment[p_partner] = rack;
                                used_products[p_partner] = true;
                            }
                        }
                    }
                } else {
                    cur_rack_freq++;
                    if (cur_rack_freq >= (int)solution.data.aisles_racks[cur_aisle_freq].size()) {
                        cur_rack_freq = 0;
                        cur_aisle_freq++;
                    }
                }
            }
        }

        // --- PHASE C : Placement du surplus de never_used (Après la famille courante) ---
        // On cherche d'abord des racks impairs, puis n'importe quel rack
        for (int p_left : leftover_nu) {
            bool assigned = false;
            // On peut réutiliser cur_aisle_freq pour continuer le remplissage dense
            while (!assigned && cur_aisle_freq < solution.data.num_aisles) {
                int rack = solution.data.aisles_racks[cur_aisle_freq][cur_rack_freq];

                if ((int)rack_content[rack].size() < solution.data.rack_capacity[rack]) {
                    // Ici on accepte n'importe quel rack car on est en phase "surplus"
                    rack_content[rack].push_back(p_left);
                    solution.assignment[p_left] = rack;
                    used_products[p_left] = true;
                    assigned = true;
                } else {
                    cur_rack_freq++;
                    if (cur_rack_freq >= (int)solution.data.aisles_racks[cur_aisle_freq].size()) {
                        cur_rack_freq = 0;
                        cur_aisle_freq++;
                    }
                }
            }
        }
    }
    return solution.assignment;
}
*/

void Heuristic_3::build_product_to_orders() {
    product_to_orders.clear();
    for (int i = 0; i < solution.data.num_orders; i++) {
        for (int product : solution.data.orders[i]) {
            product_to_orders[product].push_back(i);
        }
    }
}

void Heuristic_3::build_rack_used() {
    rack_used = vector<int>(solution.data.num_racks, 0);
    for (int p = 0; p < solution.data.num_products; p++) {
        for(int i=0 ; i<solution.data.num_products ; i++) {
            //cout << solution.assignment[i] << endl;
        }
        int r = solution.assignment[p];
        if (r >= 0 && r < solution.data.num_racks) {
            rack_used[r]++;
        }
    }
}

void Heuristic_3::build_circuit_intervals() {
    circuit_intervals = vector<pair<int, int>>(solution.data.num_circuits, {INT_MAX, INT_MIN});
    for (int c = 0; c < solution.data.num_circuits; c++) {
        for (int p : circuit_products[c]) {
            int r = solution.assignment[p];
            if (r >= 0) {
                circuit_intervals[c].first = min(circuit_intervals[c].first, r);
                circuit_intervals[c].second = max(circuit_intervals[c].second, r);
            }
        }
    }
}

void Heuristic_3::calcul_cost_and_path() {
    const auto& adj = solution.data.adjacency;
    int start = 0;
    int end = solution.data.num_racks - 1;

    solution_cost = 0;
    path.clear();
    path.resize(solution.data.num_orders);

    for (int i = 0; i < solution.data.num_orders; i++) {
        set<int> racks_set;
        for (int product : solution.data.orders[i]) {
            racks_set.insert(solution.assignment[product]);
        }

        path[i].push_back(start);
        for (int rack : racks_set) {
            path[i].push_back(rack);
        }
        path[i].push_back(end);

        for (size_t j = 0; j + 1 < path[i].size(); j++) {
            solution_cost += adj[path[i][j]][path[i][j + 1]];
        }
    }
}

long long int factoriel(int n, long long cap) {
    long long res = 1;
    for (int i = 2; i <= n; i++) {
        if (res > cap / i) return cap;
        res *= i;
        if (res >= cap) return cap;
    }
    return res;
}

unordered_map<int, vector<int>> read_product_pairs(const string& filename, int num_products) {
    unordered_map<int, vector<int>> pairs;
    ifstream read(filename);
    if (!read) {
        cout << "Erreur impossible d'ouvrir : " << filename << endl;
        abort();
    }
    string line;
    while (getline(read, line)) {
        if (line.size() == 0) continue;
        istringstream iss(line);
        int p;
        iss >> p;
        if (p < 0 || p >= num_products) continue;
        vector<int> neigh;
        int q;
        while (iss >> q) {
            if (q == -1) break; // signifie : pas de voisins
            if (q < 0 || q >= num_products) continue;
            if (q == p) continue;
            neigh.push_back(q);
        }
        pairs[p] = neigh; // vide si -1 ou rien
    }
    read.close();
    // Sécurité : tous les produits doivent exister dans la map
    for (int p = 0; p < num_products; p++) {
        if (pairs.find(p) == pairs.end()) {
            pairs[p] = vector<int>();
        }
    }
    return pairs;
}

vector<vector<int>> read_freq_prod(string filename, int num_circuit, vector<int> product_circuit) {
    /*
    Lit un fichier freq_prod_filename.txt et retourne un vecteur qui associe à 
    une famille tout les produits qui la compose dans l'ordre décroissant de leur fréquence d'apparition. 
    */
    vector<vector<int>> freq_prod(num_circuit);
    ifstream read(filename);
    if (!read) {
        cout << "Erreur impossible d'ouvrir : " << filename << endl;
        abort();
    }
    string line;
    getline(read,line); // skip header
    // lecture du fichier jusqu'à la fin du fichier :
    while (getline(read, line)){
        int id_prod; int count; double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_prod >> count >> pourcentage)) { break; } // error
        freq_prod[product_circuit[id_prod]].push_back(id_prod);
    }
    read.close();
    return freq_prod;
}

vector<vector<int>> read_never_used_products(string filename, int num_circuit, int num_products, vector<int> product_circuit){
    set<int> used;
    vector<vector<int>> never_used(num_circuit);
    ifstream read(filename);
    if (!read) {
        cout << "Erreur impossible d'ouvrir : " << filename << endl;
        abort();
    }
    string line;
    getline(read,line); // skip header
    // lecture du fichier jusqu'à la fin du fichier :
    while (getline(read, line)){
        int id_prod; int count; double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_prod >> count >> pourcentage)) { break; } // error
        used.insert(id_prod);
    }
    read.close();
    int count = 0;
    for (int p = 0; p < num_products; p++) {
        if (used.find(p) == used.end()) {
            never_used[product_circuit[p]].push_back(p);
            count++;
        }
    }
    cout << "Number of never used products ratio : " << count*100.0/num_products << " % and count : " << count << endl;
    return never_used;
}

vector<vector<int>> read_concordance_circuit(const string& filename) {
    ifstream read(filename);
    if (!read) {
        cout << "Erreur : impossible d'ouvrir " << filename << endl;
        abort();
    }
    int n;
    read >> n;
    vector<vector<int>> concord(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            read >> concord[i][j];
        }
    }
    read.close();
    return concord;
}

// concord : int (co-apparitions)
// freq_count[c] : nombre de commandes contenant le circuit c
vector<int> build_circuit_order_by_concordance_fast_weighted(const vector<int>& frequency_circuits, const vector<vector<int>>& concord, const vector<int>& freq_count) {
    int n = (int)concord.size();
    vector<int> order;
    order.reserve(n);
    vector<char> used(n, 0);
    // score cumulatif pondéré
    vector<int> score(n, 0);
    // 1) premier
    int first = frequency_circuits[0];
    order.push_back(first);
    used[first] = 1;
    // init scores
    for (int x = 0; x < n; x++) {
        score[x] = freq_count[first] * concord[first][x];
    }
    // 2) glouton
    while ((int)order.size() < n) {
        int best = -1;
        int best_score = -1;
        for (int x = 0; x < n; x++) {
            if (used[x]) continue;
            if (score[x] > best_score) {
                best_score = score[x];
                best = x;
            }
        }
        if (best == -1) break;
        order.push_back(best);
        used[best] = 1;
        // mise à jour pondérée
        for (int x = 0; x < n; x++) {
            if (used[x]) continue;
            score[x] += freq_count[best] * concord[best][x];
        }
    }
    return order;
}

void read_frequency_circuits_and_counts(const string& filename, int num_circuits, vector<int>& frequency_circuits, vector<int>& freq_count) {
    frequency_circuits.clear();
    freq_count.assign(num_circuits, 0);
    ifstream read(filename);
    if (!read) {
        cout << "Erreur impossible d'ouvrir : " << filename << endl;
        abort();
    }
    string line;
    getline(read, line); // skip header
    while (getline(read, line)) {
        if (line.empty()) continue;
        int id_circuit;
        int nb_prod_in_family;
        int count;
        double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_circuit >> nb_prod_in_family >> count >> pourcentage)) break;
        if (id_circuit < 0 || id_circuit >= num_circuits) continue;
        frequency_circuits.push_back(id_circuit);
        freq_count[id_circuit] = count;
    }
    read.close();
    // Ajouter les circuits absents (fréquence 0)
    for (int c = 0; c < num_circuits; c++) {
        if (find(frequency_circuits.begin(), frequency_circuits.end(), c) == frequency_circuits.end()) {
            frequency_circuits.push_back(c);
        }
    }
}
