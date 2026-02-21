#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"
// #include "Model.hpp"
// #include "Model2.hpp"
#include "Heur_1.hpp"
#include "Heur_2.hpp"
#include "Heur_3.hpp"
// #include "Checker.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>

#include "Utilitary.hpp"

using namespace std;

void checkCoveragePairsNever(int num_products, const unordered_map<int, vector<int>>& product_pairs, const vector<vector<int>>& never_used) {
    vector<bool> seen(num_products, false);
    int count = 0;

    // 1. We mark the keys of product_pairs (the "source" products of affinity)
    for (auto const& [p_id, partners] : product_pairs) {
        if (p_id >= 0 && p_id < num_products) {
            if (!seen[p_id]) {
                seen[p_id] = true;
                count++;
            }
        }
    }

    // 2. We mark all products in never_used
    for (const auto& circuit_vec : never_used) {
        for (int p_id : circuit_vec) {
            if (p_id >= 0 && p_id < num_products) {
                if (!seen[p_id]) {
                    seen[p_id] = true;
                    count++;
                }
            }
        }
    }

    // 3. Report results
    if (count == num_products) {
        cout << "[OK] Tous les produits sont couverts par Pairs + NeverUsed." << endl;
    } else {
        cout << "[ALERTE] " << (num_products - count) << " produits sont manquants dans Pairs + NeverUsed !" << endl;
        cout << "IDs manquants : ";
        for (int i = 0; i < num_products; i++) {
            if (!seen[i]) cout << i << " ";
        }
        cout << endl;
    }
}

void checkFreqCoverage(int num_products, const vector<vector<int>>& freq_products, const vector<vector<int>>& never_used) {
    vector<bool> seen(num_products, false);
    int count = 0;

    for (const auto& circuit_vec : freq_products) {
        for (int p_id : circuit_vec) {
            if (p_id >= 0 && p_id < num_products) {
                if (!seen[p_id]) {
                    seen[p_id] = true;
                    count++;
                }
            }
        }
    }

    for (const auto& circuit_vec : never_used) {
        for (int p_id : circuit_vec) {
            if (p_id >= 0 && p_id < num_products) {
                if (!seen[p_id]) {
                    seen[p_id] = true;
                    count++;
                }
            }
        }
    }

    if (count == num_products) {
        cout << "[OK] Tous les produits sont dans freq_products." << endl;
    } else {
        cout << "[INFO] freq_products contient " << count << "/" << num_products << " produits." << endl;
        cout << "Produits non présents (souvent ceux de never_used) : ";
        for (int i = 0; i < num_products; i++) {
            if (!seen[i]) cout << i << " ";
        }
        cout << endl;
    }
}

string buildSolutionPath(const string& warehouse_dir, const string& method_name) {
    return warehouse_dir + "/solutions/rack_product_assignment_" + method_name + ".txt";
}

void writeAndCheckSolution(const WarehouseInstance& data, WarehouseSolution& sol, const string& warehouse_dir, const string& method_name) {
    string sol_file = buildSolutionPath(warehouse_dir, method_name);

    cout << "\n----------------------------------" << endl;
    cout << "ECRITURE + CHECK : " << method_name << endl;
    cout << "----------------------------------" << endl;

    cout << "Écriture de : solution -> " << sol_file << endl;
    sol.write(sol_file);

    cout << "Test de : Checker" << endl;
    // Checker checker(data, sol_file);
    // bool check = checker.check();

    // if (check) {
    //     cout << "[OK] Solution is valid" << endl;
    //     cout << "COST : " << checker.calculateCost() << endl;
    // } else {
    //     cout << "[ERREUR] Solution is not valid" << endl;
    // }

    string command = "python3 tools/checker.py " + warehouse_dir + " " + method_name;
    int result = system(command.c_str());
}

string short_instance_name(const string& instance_name) {
    if (instance_name == "warehouse_toy") return "toy";
    if (instance_name == "warehouse_big_market") return "market";
    if (instance_name == "warehouse_big_category") return "category";
    if (instance_name == "warehouse_big_family") return "family";
    return instance_name;  // fallback
}

void writeMethodReport(const string& instance_short, const string& method_short, long long cost, long long time_seconds,
                       const bool aer_switch = false) {
    // nom fichier : H3_market.txt par exemple
    string filename = method_short + "_" + instance_short + ".txt";

    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "[ERREUR] Impossible d'écrire le fichier : " << filename << endl;
        return;
    }
    if (method_short == "H3") {
        out << "Instance Methode Cout Temps Aer_switch\n";
        out << instance_short << " " << method_short << " " << cost << " " << time_seconds << " " << aer_switch << "\n";
    } else {
        out << "Instance Methode Cout Temps\n";
        out << instance_short << " " << method_short << " " << cost << " " << time_seconds << "\n";
    }

    out.close();
    cout << "[INFO] Résultats écrits dans : " << filename << endl;
}

int main() {
    srand(time(nullptr));

    // Choose instance to solve
    // const string INSTANCE_NAME = "warehouse_toy";
    // const string INSTANCE_NAME = "warehouse_big_market";
    // const string INSTANCE_NAME = "warehouse_big_category";
    const string INSTANCE_NAME = "warehouse_big_family";

    const string INSTANCE_SHORT = short_instance_name(INSTANCE_NAME);

    // Choose method
    // bool test_M1 = false;
    // bool test_M2 = false;
    bool test_H1 = false;
    bool test_H2 = true;
    bool test_H3 = true;

    const string WAREHOUSE_DIR = "warehouses/" + INSTANCE_NAME;
    const string ANALYSE_DIR = "analyse_data/";

    // Files in analyse_data generated by python script
    const string FILE_FREQ_PROD = ANALYSE_DIR + "freq_prod_" + INSTANCE_NAME + ".txt";
    const string FILE_FREQ_CIRCUIT = ANALYSE_DIR + "freq_circuit_" + INSTANCE_NAME + ".txt";
    const string FILE_CONCORD_PROD = ANALYSE_DIR + "concord_prod_same_circuit_" + INSTANCE_NAME + ".txt";
    const string FILE_CONCORD_CIRCUIT = ANALYSE_DIR + "concord_circuit_" + INSTANCE_NAME + ".txt";

    // ---------------------------------------------------------------------
    // READ DATA
    // ---------------------------------------------------------------------
    cout << "Lecture de : instance -> " << WAREHOUSE_DIR << endl;
    WarehouseLoader loader(WAREHOUSE_DIR);
    WarehouseInstance data = loader.loadAll();

    // ---------------------------------------------------------------------
    // READ ANALYSIS FILES (for heuristics)
    // ---------------------------------------------------------------------
    cout << "Lecture de : product_pairs -> " << FILE_CONCORD_PROD << endl;
    unordered_map<int, vector<int>> product_pairs = readProductPairs(FILE_CONCORD_PROD, data.num_products);

    cout << "Lecture de : freq_products -> " << FILE_FREQ_PROD << endl;
    vector<vector<int>> freq_products = readFreqProd(FILE_FREQ_PROD, data.num_circuits, data.product_circuit);

    cout << "Lecture de : never_used -> " << FILE_FREQ_PROD << endl;
    vector<vector<int>> never_used = readNeverUsedProducts(FILE_FREQ_PROD, data.num_circuits, data.num_products, data.product_circuit);

    vector<int> frequency_circuits;
    vector<int> freq_count;
    cout << "Lecture de : frequency_circuits + counts -> " << FILE_FREQ_CIRCUIT << endl;
    readFrequencyCircuitsAndCounts(FILE_FREQ_CIRCUIT, data.num_circuits, frequency_circuits, freq_count);

    cout << "Lecture de : concordance circuits -> " << FILE_CONCORD_CIRCUIT << endl;
    vector<vector<int>> concord = readConcordanceCircuit(FILE_CONCORD_CIRCUIT);

    cout << "Exécution de checkCoveragePairsNever " << endl;
    checkCoveragePairsNever(data.num_products, product_pairs, never_used);

    cout << "Exécution de checkFreqCoverage " << endl;
    checkFreqCoverage(data.num_products, freq_products, never_used);

    // ---------------------------------------------------------------------
    // PREPARATION : circuit order + initial solution
    // ---------------------------------------------------------------------
    cout << "Exécution de : build_circuit_order_by_concordance_fast_weighted" << endl;
    vector<int> order_circuit = buildCircuitOrderByConcordanceFastWeighted(frequency_circuits, concord, freq_count);

    cout << "Exécution de : buildInitialSolution" << endl;
    vector<int> initial_assignment = buildInitialSolution(data, frequency_circuits);
    WarehouseSolution initial_sol(data, initial_assignment);

    // // ---------------------------------------------------------------------
    // // TEST MODEL 1
    // // ---------------------------------------------------------------------
    // if (test_M1) {
    //     cout << "\n==========================" << endl;
    //     cout << "TEST MODEL 1" << endl;
    //     cout << "==========================" << endl;

    //     cout << "Exécution de : Model M1.solve()" << endl;
    //     Model M1(data);

    //     WarehouseSolution sol = M1.solve();

    //     cout << "Affichage : solution Model 1" << endl;
    //     sol.print();

    //     cout << "Résultat : coût Model 1 = " << calculate_cost(sol) << endl;

    //     writeAndCheckSolution(data, sol, WAREHOUSE_DIR, "M1");
    //     write_method_report(INSTANCE_SHORT, "M1", cost, duration);

    // }

    // // ---------------------------------------------------------------------
    // // TEST MODEL 2
    // // ---------------------------------------------------------------------
    // if (test_M2) {
    //     cout << "\n==========================" << endl;
    //     cout << "TEST MODEL 2" << endl;
    //     cout << "==========================" << endl;

    //     cout << "Exécution de : génération circuit_sequence" << endl;
    //     vector<int> circuit_sequence(data.num_circuits);
    //     for (int f = 0; f < data.num_circuits; f++) {
    //         circuit_sequence[f] = f;
    //     }

    //     cout << "Exécution de : Model2 M2.solve2()" << endl;
    //     Model2 M2(data, 5, circuit_sequence);
    //     WarehouseSolution sol2 = M2.solve2();

    //     cout << "Résultat : coût Model 2 = " << calculate_cost(sol2) << endl;

    //     writeAndCheckSolution(data, sol2, WAREHOUSE_DIR, "M2");
    //     write_method_report(INSTANCE_SHORT, "M1", cost, duration);

    // }

    // ---------------------------------------------------------------------
    // TEST HEURISTIQUE 1
    // ---------------------------------------------------------------------
    if (test_H1) {
        cout << "Exécution de : Heur_1(data, 3)" << endl;

        auto start_time = chrono::high_resolution_clock::now();
        WarehouseSolution sol = Heur_1(data, 3);
        auto end_time = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

        long long cost = calculateCost(sol);
        cout << "Résultat : coût H1 = " << cost << endl;
        cout << "Temps d'exécution : " << duration << " secondes" << endl;

        writeAndCheckSolution(data, sol, WAREHOUSE_DIR, "heur_1");
        writeMethodReport(INSTANCE_SHORT, "H1", cost, duration);
    }

    // ---------------------------------------------------------------------
    // TEST HEURISTIQUE 3
    // ---------------------------------------------------------------------

    bool has_sol_H3 = false;  // Necessaire à pour H2
    Heuristic_3 H3(initial_sol);
    if (test_H3) {
        cout << "\n==========================" << endl;
        cout << "TEST HEURISTIQUE 3" << endl;
        cout << "==========================" << endl;

        cout << "Exécution de : buildInitialSolution (H3)" << endl;
        vector<int> assignment_H3 = H3.buildInitialSolution(order_circuit, freq_products, product_pairs, never_used);

        has_sol_H3 = true;

        long long cost = calculateCost(H3.solution);
        cout << "Résultat : coût avant improve = " << cost << endl;

        int max_iter_circuit = 50000;
        int max_iter_without_improv = 100;
        bool allow_aeration_switch = true;
        cout << "Exécution de : H3.improve(" << max_iter_circuit << ", " << max_iter_without_improv << ")" << endl;
        auto start_time = chrono::high_resolution_clock::now();
        H3.improve(max_iter_circuit, max_iter_without_improv, allow_aeration_switch);
        auto end_time = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();
        cout << "Temps d'exécution : " << duration << " secondes" << endl;

        int cost_2 = calculateCost(H3.solution);
        cout << "Résultat : coût après improve = " << cost_2 << endl;

        writeAndCheckSolution(data, H3.solution, WAREHOUSE_DIR, "heur_3");

        writeMethodReport(INSTANCE_SHORT + "Aeration", "H3", cost_2, duration, allow_aeration_switch);
    }

    // ---------------------------------------------------------------------
    // TEST HEURISTIQUE 2
    // ---------------------------------------------------------------------
    if (test_H2) {
        cout << "\n==========================" << endl;
        cout << "TEST HEURISTIQUE 2" << endl;
        cout << "==========================" << endl;

        // Choose start
        bool start_from_H3 = false;
        if (start_from_H3 && has_sol_H3) {
            cout << "[INFO] H2 démarre depuis la solution H3." << endl;
            initial_sol = H3.solution;
        } else {
            cout << "[INFO] H2 démarre depuis la solution initiale." << endl;
        }

        cout << "Exécution de : initialisation H2" << endl;
        Heuristic_2 H2(initial_sol);

        int max_iter_circuit = 50000;
        int max_iter_without_improv = 100;
        cout << "Exécution de : H2.improve(" << max_iter_circuit << ", " << max_iter_without_improv << ")" << endl;
        auto start_time = chrono::high_resolution_clock::now();
        H2.improve(max_iter_circuit, max_iter_without_improv);
        auto end_time = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();
        cout << "Temps d'exécution : " << duration << " secondes" << endl;

        cout << "Résultat : coût après H2 = " << H2.solution_cost << endl;

        writeAndCheckSolution(data, H2.solution, WAREHOUSE_DIR, "heur_2");
        writeMethodReport(INSTANCE_SHORT + "InitH2", "H2", H2.solution_cost, duration);
    }

    return 0;
}

// - Accepter les switch avec l'aération
// - Prendre des couples de produits les plus frequemments ensemble pour faire les switch (et pour la solution initiale)
// - Faire un liste Tabou qui interdit de faire des switch entre les produits d'un même couple pendant un certain nombre d'itérations

// - Faire un liste de produits ordonné par fréquence d'apparition dans toutes les commandes pour chaque produits.
// - Pour placer les produits dans la sol initiale, à famille fixé, prendre le produit le plus fréquemment commandé (dans l'ensemble des commandes) et
// le placer.
//  Regarder ensuite les produits les plus frequemment commandé avec ce produit et les placer jusqu'à complétion de la capacité du rack.
//  Stocker l'ensemble des produits qui ont déjà été placés. Placer dans le rack suivant le produit le plus fréquemment commandé suivant. S'il a déjà
//  été placé, prendre le suivant et ainsi de suite.