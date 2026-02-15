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

vector<int> Heuristic_3::initial_solution3(const vector<int>& frequency_circuits, vector<vector<int>> freq_products, unordered_map<int, vector<int>> product_pairs, vector<vector<int>> never_used) {
    /*
    Génère une solution initiale en assignant les produits des circuits les plus fréquents aux racks les plus proches de la sortie. 
    L'aération des positionné en bas à gauche de chaque allée.
    Les produits sont regroupées par pairs de produits les plus frequemment utilisés.
    */
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
