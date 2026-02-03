
#include "Model_2.hpp"

#include "gurobi_c++.h"

using namespace std;

void Model2::choose_orders(int seed = 0) {

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, data.num_orders - 1);

    unordered_set<int> indices;
    indices.reserve(num_orders);

    while (indices.size() < num_orders) {
        indices.insert(dist(gen));
    }

    for (int c : indices) {
        cout << "Order " << c << endl;
        orders.push_back(data.orders[c]);
    }
}

void Model2::calcul_racks_circuits() {
    
    vector<int> new_rack_capacity = New_rack_capacity(data);

    racks_circuits.resize(data.num_circuits);

    int rack = 0;
    
    for (int f = 0; f < data.num_circuits; f++) {
        racks_circuits[f].resize(2);
        while (new_rack_capacity[rack] < 1) rack++;
        racks_circuits[f][0] = rack;

        int nProducts = circuits[f].size();
        while (nProducts > 0) {
            if (nProducts <= new_rack_capacity[rack]) {
                new_rack_capacity[rack] -= nProducts;
                nProducts = 0;
            } else {
                nProducts -= new_rack_capacity[rack];
                rack++;
            }
        }
        racks_circuits[f][1] = rack;

    }
}

Model2::Model2(const WarehouseInstance& Data, int Num_orders) : Model(Data), num_orders(Num_orders) {
    circuits.resize(data.num_circuits);
    for (int j = 0; j < data.num_products; j++){
        circuits[data.product_circuit[j]].push_back(j);
    }

    choose_orders();
    calcul_racks_circuits();

}

void Model2::print_circuits() {
    for (int c = 0; c < data.num_circuits; c++) {
        cout << "Circuit " << c << " : ";
        for (int j : circuits[c]) {
            cout << j << ", ";
        }
        cout << endl;
    }
}

vector<int> Model2::assignment_real() {
    vector<int> new_rack_capacity = New_rack_capacity(data);
    // The products are placed in order
    vector<int> assignment (data.num_products, 0);
    int current_rack = 1;
    for (int current_circuit = 0; current_circuit < data.num_circuits; current_circuit++/*: circuit_sequence*/) {
        for (int j : circuits[current_circuit]) {
            
            while (new_rack_capacity[current_rack] == 0) current_rack++;
            
            assignment[j] = current_rack;
            new_rack_capacity[current_rack]--;
            
        }
    }
    return assignment;
}


WarehouseSolution Model2::solve2() {
    GRBEnv env(true);
    //env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model(env);

    ///////////////////////////
    /////// Variables /////////
    ///////////////////////////
cout << "Creating variables x" << endl;
    // Variables Xij
    vector<vector<GRBVar>> x(data.num_racks, vector<GRBVar> (data.num_products));
    for (int j = 0; j < data.num_products; j++){
        for (int i = racks_circuits[data.product_circuit[j]][0];
            i < racks_circuits[data.product_circuit[j]][1] + 1; i++){
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + to_string(i) + "_" + to_string(j));
        }
    }

    
    cout << "Creating variables z" << endl;
    // Variables Zcii'
    vector<vector<vector<GRBVar>>> z(num_orders, vector<vector<GRBVar>> (data.num_racks, vector<GRBVar> (data.num_racks)));
    for (int c = 0; c < num_orders; c++){
        for (int i = 0; i < data.num_racks; i++){
            for (int j = i + 1; j < data.num_racks; j++){
                z[c][i][j] = model.addVar(0.0, 1.0, data.adjacency[i][j], GRB_BINARY, "z_" + to_string(c) + "_" + to_string(i) + "_" + to_string(j));
            }
        }
    }


    ///////////////////////////
    ////// Contraintes ////////
    ///////////////////////////
    cout << "Creating constraints 2" << endl;
    // Chaque objet assigné à un rack
    for (int j = 0; j < data.num_products; j++) {
        GRBLinExpr lhs = 0;
        for (int i = racks_circuits[data.product_circuit[j]][0];
        i < racks_circuits[data.product_circuit[j]][1] + 1; i++) {
            lhs += x[i][j];
        }
        model.addConstr(lhs == 1, "assignment_" + to_string(j));
    }

    cout << "Creating constraints 3" << endl;
    // Capacité des racks
    vector<int> new_rack_capacity = New_rack_capacity(data);
    for (int i = 1; i < data.num_racks - 1; i++) {
        GRBLinExpr lhs = 0;
        for (int j = 0; j < data.num_products; j++) {
            if (i >= racks_circuits[data.product_circuit[j]][0] &&
        i <= racks_circuits[data.product_circuit[j]][1])
            lhs += x[i][j];
        }
        model.addConstr(lhs <= new_rack_capacity[i], "capacity_" + to_string(i));
    }

    cout << "Creating constraints 8" << endl;
    // La commande passe par les racks contenant ses produits
    for (int c = 0; c < num_orders; c++) {
        for (int j : orders[c]) {
            for (int i = racks_circuits[data.product_circuit[j]][0];
        i < racks_circuits[data.product_circuit[j]][1] + 1; i++) {
                GRBLinExpr lhs = 0;
                for (int ii = 0; ii < i; ii++) {
                    lhs += z[c][ii][i];
                }
                model.addConstr(lhs >= x[i][j], "order_" + to_string(c));
            }
        }
    }

    cout << "Creating constraints 10" << endl;
    // Contraintes de flot
    for (int c = 0; c < num_orders; c++) {vector<int> new_rack_capacity = New_rack_capacity(data);
        GRBLinExpr lhs = 0;
        for (int i = 1; i < data.num_racks - 1; i++) {
            lhs += z[c][0][i];
        }
        model.addConstr(lhs == 1);
    }

    cout << "Creating constraints 11" << endl;
    for (int c = 0; c < num_orders; c++) {
        GRBLinExpr lhs = 0;
        for (int i = 1; i < data.num_racks - 1; i++) {
            lhs += z[c][i][data.num_racks - 1];
        }
        model.addConstr(lhs == 1);
    }

    cout << "Creating constraints 9" << endl;
    for (int c = 0; c < num_orders; c++) {
        for (int ii = 1; ii < data.num_racks - 1; ii++) {
            GRBLinExpr lhs = 0;
            for (int i = 0; i < ii; i++) {
                lhs += z[c][i][ii];
            }
            for (int i = ii + 1; i < data.num_racks; i++) {
                lhs -= z[c][ii][i];
            }
            model.addConstr(lhs == 0);
        }
    }
    
    /*vector<int> start_assignment = assignment_real();
    for (int j : start_assignment) {
        cout << j << endl;
    }
    for (int j = 0; j < data.num_products; j++) {
    
        x[start_assignment[j]][j].set(GRB_DoubleAttr_Start, 1.0);
        
    }*/
    
    model.set(GRB_DoubleParam_TimeLimit, 1800);
    model.optimize();

    // Récupération de la solution
    vector<int> assignment(data.num_products, 0);
    for (int j = 0; j < data.num_products; j++) {
        for (int i = racks_circuits[data.product_circuit[j]][0];
        i < racks_circuits[data.product_circuit[j]][1] + 1; i++) {
            if (x[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                assignment[j] = i;
                break;
            }
        }
    }
    return WarehouseSolution(data, assignment);
}
