#include "Heur_2.hpp"
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

static std::mt19937 rng((std::random_device())());

Heuristic_2::Heuristic_2() {} // default constructor

Heuristic_2::Heuristic_2(const WarehouseSolution& initial_solution) {
    solution.data = initial_solution.data;
    solution.assignment = initial_solution.assignment;
    circuit_products = build_product_in_circuit(solution.data);
    build_product_to_orders();
    build_rack_used();
    build_circuit_intervals();
    calcul_cost_and_path();
}

void Heuristic_2::build_product_to_orders() { 
    /*
    Associe à chaque produit l'ensemble des commandes auquel il est associé.
    */
    product_to_orders.clear();
    for (int i = 0; i < solution.data.num_orders; i++) {
        for (int product : solution.data.orders[i]) {
            product_to_orders[product].push_back(i);
        }
    }
}

void Heuristic_2::build_rack_used() {
    /*
    Construit le vecteur indiquant le nombre de produits actuellement assignés à chaque rack.
    */
    rack_used = vector<int>(solution.data.num_racks, 0);
    for (int p = 0; p < solution.data.num_products; p++) {
        int r = solution.assignment[p];
        if (r >= 0 && r < solution.data.num_racks) {
            rack_used[r]++;
        }
    }
}

void Heuristic_2::build_circuit_intervals() {
    /*
    Détermine l'intervalle [min_rack, max_rack] pour chaque circuit basé sur l'assignement actuel.
    */
    circuit_intervals = vector<pair<int, int>>(solution.data.num_circuits, {INT_MAX, INT_MIN});
    for (int c = 0; c < solution.data.num_circuits; c++) {
        for (int p : circuit_products[c]) {
            int r = solution.assignment[p];
            if (r >= 0) {
                circuit_intervals[c].first = min(circuit_intervals[c].first, r);
                circuit_intervals[c].second = max(circuit_intervals[c].second, r);
            }
        }
        // Si le circuit est vide ou non assigné, on ignore (min > max)
    }
}

void Heuristic_2::calcul_cost_and_path() {

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

void Heuristic_2::improve(const int max_attempts_per_circuit, const int stagnation_threshold) {
    /*
    Heuristique d'amélioration intra-circuit : recherche locale par échanges (swaps) de deux produits au sein de l'intervalle de racks de leur circuit.
    Seuls les coûts des commandes affectées sont recalculés pour évaluer le delta.
    */

    for (int c = 0; c < solution.data.num_circuits; c++) {
        if (circuit_products[c].empty() || circuit_products[c].size() < 2) continue;

        int min_r = circuit_intervals[c].first;
        int max_r = circuit_intervals[c].second;
        if (min_r > max_r) continue; // Circuit non assigné

        int attempts = 0;
        int stagnant = 0;

        while (attempts < max_attempts_per_circuit && stagnant < stagnation_threshold) {
            attempts++;
            stagnant++;

            // Sélection aléatoire de deux produits distincts dans le circuit
            size_t idx1 = rng() % circuit_products[c].size();
            size_t idx2 = rng() % circuit_products[c].size();
            if (idx1 == idx2) continue; // Assurer distincts

            int p1 = circuit_products[c][idx1];
            int p2 = circuit_products[c][idx2];
            int old_r1 = solution.assignment[p1];
            int old_r2 = solution.assignment[p2];

            if (old_r1 == old_r2) continue; // Pas de swap si même rack

            // Calcul du delta coût seulement sur les commandes affectées par p1 ou p2
            set<int> affected_orders_set;
            for (int o : product_to_orders[p1]) {
                affected_orders_set.insert(o);
            }
            for (int o : product_to_orders[p2]) {
                affected_orders_set.insert(o);
            }
            if (affected_orders_set.empty()) continue;

            long long delta = 0;
            for (int o : affected_orders_set) {
                // Coût ancien pour cette commande
                const vector<int>& old_path = path[o];
                long long old_order_cost = 0;
                for (size_t j = 0; j + 1 < old_path.size(); j++) {
                    old_order_cost += solution.data.adjacency[old_path[j]][old_path[j + 1]];
                }

                // Nouveau set de racks pour cette commande
                set<int> new_racks_set;
                for (int prod : solution.data.orders[o]) {
                    int rr;
                    if (prod == p1) {
                        rr = old_r2;
                    } else if (prod == p2) {
                        rr = old_r1;
                    } else {
                        rr = solution.assignment[prod];
                    }
                    new_racks_set.insert(rr);
                }

                // Nouveau path : start + sorted racks + end
                vector<int> new_path_vec;
                new_path_vec.push_back(0);
                for (int rr : new_racks_set) {
                    new_path_vec.push_back(rr);
                }
                new_path_vec.push_back(solution.data.num_racks - 1);

                // Nouveau coût
                long long new_order_cost = 0;
                for (size_t j = 0; j + 1 < new_path_vec.size(); j++) {
                    new_order_cost += solution.data.adjacency[new_path_vec[j]][new_path_vec[j + 1]];
                }

                delta += new_order_cost - old_order_cost;
            }

            // Pas besoin de vérifier l'aération ou les capacités car le swap ne change pas le nombre de produits par rack/allée

            // Si amélioration, accepter le swap
            if (delta < 0) {
                // Mise à jour de l'assignement
                solution.assignment[p1] = old_r2;
                solution.assignment[p2] = old_r1;

                // rack_used inchangé car swap équilibre les counts

                // Mise à jour du coût global
                solution_cost += delta;

                // Mise à jour des paths des commandes affectées
                for (int o : affected_orders_set) {
                    set<int> updated_racks;
                    for (int prod : solution.data.orders[o]) {
                        updated_racks.insert(solution.assignment[prod]);
                    }
                    vector<int> updated_path;
                    updated_path.push_back(0);
                    for (int rr : updated_racks) {
                        updated_path.push_back(rr);
                    }
                    updated_path.push_back(solution.data.num_racks - 1);
                    path[o] = updated_path;
                }

                // Reset du compteur de stagnation car amélioration trouvée
                stagnant = 0;
            }
        }
    }
}

vector<vector<int>> build_product_in_circuit(const WarehouseInstance& data) {
    /*
    Associe à chaque circuit l'ensemble des produits qui le compose.
    */
    vector<vector<int>> product_in_circuit = vector<vector<int>> (data.num_circuits, vector<int>(0));
    for (int i = 0; i < data.num_products; i++) {
        product_in_circuit[data.product_circuit[i]].push_back(i);
    }
    return product_in_circuit;
}


vector<int> read_frequency_circuits(string filename, int num_circuits) {
    /*
    Lit le fichier de fréquence des circuits et retourne un vecteur d'indice de circuit trier par ordre décroissant de la fréquence d'apparition.
    */
    vector<int> frequency_circuits;
    ifstream read(filename);
    if (!read) {
        cout << "Erreur impossible d'ouvrir : " << filename << endl;
        abort();
    }
    string line;
    getline(read,line); // skip header
    // lecture du fichier jusqu'à la fin du fichier :
    while (getline(read, line)){
        int id_circuit; int nb_prod_in_family; int count; double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_circuit >> nb_prod_in_family >> count >> pourcentage)) { break; } // error
        frequency_circuits.push_back(id_circuit);
    }
    read.close();
    
    // Ajout de tout les circuits qui n'apparaissent pas dans le fichier (fréquence 0) :
    for (int i = 0; i < num_circuits; i++) {
        if (find(frequency_circuits.begin(), frequency_circuits.end(), i) == frequency_circuits.end()) {
            frequency_circuits.push_back(i);
        }
    }
    return frequency_circuits;
}

vector<int> new_rack_capacity(const WarehouseInstance& data) {
    /*
    Modify the capacity of racks by adding aeration randomly in each aisle.
    */
    vector<int> rack_capacity = data.rack_capacity;
    unordered_map<int, int> aeration_rack;

    for (int i = 0; i < data.num_aisles; i++) {
        int aisle_capacity = 0;
        for (int rack : data.aisles_racks[i]) {
            aisle_capacity += data.rack_capacity[rack];
        }
        int nb_aeration_rack = ceil(aisle_capacity*data.aeration_rate/100.); // Renvoie le plus petit entier supérieur ou égal à aisle_capacity*data.aeration_rate/100.
        while (nb_aeration_rack > 0) {
            int random_rack_index = rand() % data.aisles_racks[i].size();
            int random_rack = data.aisles_racks[i][random_rack_index];
            if (rack_capacity[random_rack] > 0) {
                rack_capacity[random_rack] -= 1;
                nb_aeration_rack -= 1;
                aeration_rack[random_rack] += 1;
            }
        }
    }
    // cout << "Racks d'aération : ";
    // for (auto& pair : aeration_rack) {
    //     cout << pair.first << " : " << pair.second << " aeration " << endl;
    // }
    // cout << endl;
    return rack_capacity;
}

//\\ LES NUMEROS DES RACKS COMMENCENT A 1 et fini à 1599 DONC n-2 est le numéro du dernier rack
vector<int> initial_solution(const WarehouseInstance& data, const vector<int>& frequency_circuits){
    /*
    Génère une solution initiale en assignant les produits des circuits les plus fréquents aux racks les plus proches de la sortie.
    */
    vector<int> assignment (data.num_products,-1);
    vector<int> rack_capacity = new_rack_capacity(data); // add randomly aeration in the racks of each aisle
    vector<int> rack_content (data.num_racks-1, 0);      // rack_content[i] = nombre de produits assigné au rack i   
    vector<vector<int>> product_in_circuit = build_product_in_circuit(data); 
   
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

    return assignment;
}
