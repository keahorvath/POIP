#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"
#include "Model.hpp"
#include "Model_2.hpp"
#include "Heur_1.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouses/warehouse_big_category");
    WarehouseInstance data = loader.loadAll();

    
    // TEST MODÈLE
    /*
    Model M1(data);
    WarehouseSolution sol = M1.solve();
    sol.print();
    cout << "modèle : " << calculate_cost(sol) << endl;
    string fsol = "../warehouses/warehouse_toy/solutions/rack_product_assignment_modèle.txt";
    sol.write(fsol);
    */

    // TEST GLOUTON
    /*
    vector<vector<int>> fam(data.num_circuits, vector<int>(0));
    for (int i=0 ; i<data.num_products ; i++) {
        fam[data.product_circuit[i]].push_back(i);
    }

    WarehouseSolution sol = Glouton(data, fam);
    sol.print();
    */

    // TEST HEUR_1 
    /*
    cout << "avant heur" << endl;

    WarehouseSolution sol = Heur_1 (data, 10);
    // sol.print();
    cout << "caca  " << calculate_cost(sol) << endl;
    string fsol = "../warehouses/warehouse_big_category/solutions/rack_product_assignment_heur_1.txt";
    sol.write(fsol);
    */

    // TEST MODELE 2
    
    
    vector<int> circuit_sequence (data.num_circuits);
    for (int f = 0; f < data.num_circuits; f++) {
        circuit_sequence[f] = f;
    }
    Model2 M2(data, 5, circuit_sequence);
    
    //WarehouseSolution sol1 = M1.solve();
    WarehouseSolution sol2 = M2.solve2();


    //cout << "Cost M1: " << calculate_cost(sol1) << endl;
    cout << "Cost M2: " << calculate_cost(sol2) << endl;
    
    return 0;
}
