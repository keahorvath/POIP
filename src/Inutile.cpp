#include "Inutile.hpp"
#include "gurobi_c++.h"
using namespace std;

vector<vector<int>> init_length (const WarehouseInstance& data) {

    vector<vector<int>> length (data.num_circuits + 1, vector<int> (data.num_circuits + 1));

    for (auto& order : data.orders) {

        unordered_set<int> circuits;
        for (int product : order) {
            //cout << "produit " << product << " : circuit " << data.product_circuit[product] << endl;
            circuits.insert(data.product_circuit[product] + 1);     // On indexe les familles à partir de 1
        }

        vector<int> presents;
        int n = 0;
        for (int circuit : circuits) {
            n++;
            presents.push_back(circuit);
        }
        
        for (int i = 0; i < n; i++) {
            length[0][presents[i]]++;       // Nombre de fois où la famille est présente
            for (int j = i + 1; j < n; j++) {
                length[presents[i]][presents[j]]++;
                length[presents[j]][presents[i]]++;
            }
        }

    }

    return length;
}

vector<int> ordre_circuits (const WarehouseInstance& data) {

    vector<vector<int>> length = init_length(data);

    int C = data.num_circuits + 1;

    GRBEnv env(true);
    //env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model(env);

    // Variables Xij
    vector<vector<GRBVar>> x(C, vector<GRBVar> (C));
    for (int i=0; i<C; i++) {
        for (int j=0; j<C; j++) {
            if (i!=j)
            x[i][j] = model.addVar(0.0, 1.0, -length[i][j], GRB_BINARY, "x_" + to_string(i) + "_" + to_string(j));
        }
    }

    // Variables Uj
    vector<GRBVar> u(C);
    for (int j = 1; j < C; j++) {
        u[j] = model.addVar(1.0, data.num_circuits, 0.0, GRB_INTEGER, "u_" + to_string(j));
    }
    u[0] = model.addVar(0.0, 0.0, 0.0, GRB_INTEGER, "u_0");

    // Contraintes cycle
    for (int i = 0; i < C; i++) {
        GRBLinExpr lhs1 = 0;
        GRBLinExpr lhs2 = 0;
        for (int j = 0 ; j < C; j++) {
            if (i!=j) {
                lhs1 += x[i][j];
                lhs2 += x[j][i];
            }
        } 
        model.addConstr(lhs1 == 1);
        model.addConstr(lhs2 == 1);
    }

    // Contraintes MTZ
    model.addConstr(u[0] == 0);
    for (int i = 0; i < C; i++) {
        for (int j = 1; j < C; j++) {
            if (i != j)
            model.addConstr(u[j] >= u[i] + 1 - (C-1) * (1 - x[i][j]));
        }
    } 

    model.set(GRB_DoubleParam_TimeLimit, 600);
    model.optimize();

    vector<int> sol(C-1);
    for (int j = 1; j < C; j++) {
        sol[u[j].get(GRB_DoubleAttr_X) - 1] = j - 1;
    }

    return sol;
}
 