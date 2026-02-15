#include <vector>

#include "Model.hpp"
#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    // Model 1
    Model M1(data);
    WarehouseSolution sol = M1.solve();
    sol.print();
    string fsol = "../solutions/toy.txt";
    sol.write(fsol);

    // Heur_2
    vector<int> frequency_circuits = read_frequency_circuits("freq_circuit_toy_instance.txt", data.num_circuits)
    WarehouseSolution initial_sol(data, sol_initial);
    Heuristic_2 H2(initial_sol);
    H2.improve(5000, 1000);
    H2.solution.write("../solutions/rack_product_assignment);
        
    // Heur_3
    vector<vector<int>> freq_products = read_freq_prod("freq_prod_toy_instance.txt", data.num_circuits, data.product_circuit);
    unordered_map<int, vector<int>> product_pairs = read_product_pairs("concord_prod_same_circuit_toy.txt", data.num_products);
    vector<vector<int>> never_used = read_never_used_products("freq_prod_big_market.txt", data.num_circuits, data.num_products, data.product_circuit);
    vector<int> sol_initial = initial_solution(data, frequency_circuits);
    WarehouseSolution initial_sol(data, sol_initial);
    Heuristic_3 heuristic_3(initial_sol);
    heuristic_3.initial_solution3(frequency_circuits, freq_products, product_pairs, never_used);
    string fsol = "../solutions/rack_product_assignment_heur_3.txt";
    heuristic_3.solution.write(fsol);
    
    Checker checker(data, "../solutions/toy.txt");
    bool check = checker.check();
    if (check) {
        cout << "Solution is valid" << endl;
        cout << "COST : " << checker.calculateCost() << endl;
    } else {
        cout << " Solution is not valid" << endl;
    }

    return 0;
}


