
#include "WarehouseInstance.hpp"
#include "gurobi_c++.h"

using namespace std;

void Modele::solve(){

    GRBEnv env(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model(env);

    ///////////////////////////
    /////// Variables /////////
    ///////////////////////////

    // Variables Xik
    vector<vector<GRBVar>> x(data.num_racks, vector<GRBVar> (data.num_products));
    for (int i=1; i<data.num_racks-1; i++){
        for (int j=0; j<data.num_products; j++){
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }
    }

    // Variables Yff'
    vector<vector<GRBVar>> y(data.num_circuits, vector<GRBVar> (data.num_circuits));
    for (int f=0; f<data.num_circuits; f++){
        for (int g=0; g<data.num_circuits; g++){
            if (f!=g){
                y[f][g] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
            }
        }
    }

    // Variables Zcii'
    vector<vector<vector<GRBVar>>> z(data.num_orders, vector<vector<GRBVar>> (data.num_racks, vector<GRBVar> (data.num_racks)));
    for (int c=0; c<data.num_orders; c++){
        for (int i=0; i<data.num_racks; i++){
            for (int j=i+1; j<data.num_racks; j++){
                z[c][i][j] = model.addVar(0.0, 1.0, data.adjacency[i][j], GRB_BINARY);
            }
        }
    }



    ///////////////////////////
    ////// Contraintes ////////
    ///////////////////////////


    // Chaque objet assigné à un rack
    for (int j=0; j<data.num_products; j++){
        GRBLinExpr lhs = 0;
        for (int i=1; i<data.num_racks-1; i++){
            lhs += x[i][j];
        }
        model.addConstr(lhs == 1);
    }

    // Capacité des racks
    for (int i=1; i<data.num_racks-1; i++){
        GRBLinExpr lhs = 0;
        for (int j=0; j<data.num_products; j++){
            lhs += x[i][j];
        }
        model.addConstr(lhs <= data.rank_capacity[i]);
    }

    // Aération
    for (int k=0; k<data.num_aisles; k++){
        GRBLinExpr lhsL = 0;
        int lhsR = 0;
        for (int i : data.aisles_racks[k]){
            GRBLinExpr lhs = 0;
            for (int j=0; j<data.num_products; j++){
                lhs += x[i][j];
            }
            lhsL += data.rank_capacity[i] - lhs;
            lhsR += data.rank_capacity[i] * data.aeration_rate;
        }
        lhsR = (lhsR-1)/100 + 1;
        model.addConstr(lhsL <= lhsR);
    }

    // Une famille avant l'autre
    for (int f=0; f<data.num_circuits; f++){
        for (int g=0; g<data.num_circuits; g++){
            if (f!=g){
                model.addConstr(y[f][g] + y[g][f] == 1);
            }
        }
    }

    // Cohérence sur l'ordre des familles
    for (int f=0; f<data.num_circuits; f++){
        for (int g=0; g<data.num_circuits; g++){
            if (f!=g){
                for (int h=0; h<data.num_circuits; h++){
                    if (h!=f && h!=g){
                        model.addConstr(y[f][g] + y[g][h] - 1 <= y[f][h]);
                    }
                }
            }
        }
    }

    // Cohérence sur l'ordre des produits par famille
    for (int f=0; f<data.num_circuits; f++){
        for (int g=0; g<data.num_circuits; g++){
            if (f!=g){
                for (int j=0; j<data.num_products; j++){
                    if (data.product_circuit[j] = f){
                        for (int jj=0; jj<data.num_products; jj++){
                            if (data.product_circuit[jj] = g){
                                for (int i=1; i<data.num_racks-1; i++){
                                    for (int ii=1; ii<i; ii++){
                                        model.addConstr(2 - y[f][g] - x[i][j] >= x[ii][jj]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // La commande passe par les racks contenant ses produits
    for (int c=0; c<data.num_orders; c++){
        for (int ii=1; ii<data.num_racks-1; ii++){
            for (int j : data.orders[c]){
                GRBLinExpr lhs=0;
                for (int i=0; i<ii; i++){
                    lhs += z[c][i][ii];
                }
                model.addConstr(lhs >= x[ii][j]);
            }
        }
    }

    // Contraintes de flot
    for (int c=0; c<data.num_orders; c++){
        GRBLinExpr lhs = 0;
        for (int i=1; i<data.num_racks-1; i++){
            lhs += z[c][0][i];
        }
        model.addConstr(lhs == 1);
    }

    for (int c=0; c<data.num_orders; c++){
        GRBLinExpr lhs = 0;
        for (int i=1; i<data.num_racks-1; i++){
            lhs += z[c][i][data.num_racks-1];
        }
        model.addConstr(lhs == 1);
    }

    for (int c=0; c<data.num_orders; c++){
        for (int ii=1; ii<data.num_racks-1; ii++){
            GRBLinExpr lhs = 0;
            for (int i=0; i<ii; i++){
                lhs += z[c][i][ii];
            }
            for (int i=ii+1; i<data.num_racks; i++){
                lhs -= z[c][ii][i];
            }
            model.addConstr(lhs == 0);
        }
    }

    model.optimize();

    // Récupération de la solution
    for (int j=0; j<data.num_products; j++){
        for (int i=1; i<data.num_racks-1; i++){
            if (x[i][j].get(GRB_DoubleAttr_X) > 0.5){
                assignment[j] = i;
                break;
            }
        }
    }

}


void createModel(WarehouseInstance data) {
    GRBEnv env(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model(env);

    ///////////////////////////
    /////// Variables /////////
    ///////////////////////////

    // Variables Xik
    vector<vector<GRBVar>> x(data.num_racks, vector<GRBVar>(data.num_products));
    for (int i = 1; i < data.num_racks - 1; i++) {
        for (int j = 0; j < data.num_products; j++) {
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }
    }

    // Variables Yff'
    vector<vector<GRBVar>> y(data.num_circuits, vector<GRBVar>(data.num_circuits));
    for (int f = 0; f < data.num_circuits; f++) {
        for (int g = 0; g < data.num_circuits; g++) {
            if (f != g) {
                y[f][g] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
            }
        }
    }

    // Variables Zcii'
    vector<vector<vector<GRBVar>>> z(data.num_orders, vector<vector<GRBVar>>(data.num_racks, vector<GRBVar>(data.num_racks)));
    for (int c = 0; c < data.num_orders; c++) {
        for (int i = 0; i < data.num_racks; i++) {
            for (int j = i + 1; j < data.num_racks; j++) {
                z[c][i][j] = model.addVar(0.0, 1.0, data.adjacency[i][j], GRB_BINARY);
            }
        }
    }

    ///////////////////////////
    ////// Contraintes ////////
    ///////////////////////////

    // Chaque objet assigné à un rack
    for (int j = 0; j < data.num_products; j++) {
        GRBLinExpr lhs = 0;
        for (int i = 1; i < data.num_racks - 1; i++) {
            lhs += x[i][j];
        }
        model.addConstr(lhs == 1);
    }

    // Capacité des racks
    for (int i = 1; i < data.num_racks - 1; i++) {
        GRBLinExpr lhs = 0;
        for (int j = 0; j < data.num_products; j++) {
            lhs += x[i][j];
        }
        model.addConstr(lhs <= data.rack_capacity[i]);
    }

    // Aération
    for (int k = 0; k < data.num_aisles; k++) {
        GRBLinExpr lhsL = 0;
        int lhsR = 0;
        for (int i : data.aisles_racks[k]) {
            GRBLinExpr lhs = 0;
            for (int j = 0; j < data.num_products; j++) {
                lhs += x[i][j];
            }
            lhsL += data.rack_capacity[i] - lhs;
            lhsR += data.rack_capacity[i] * data.aeration_rate;
        }
        lhsR = (lhsR - 1) / 100 + 1;
        model.addConstr(lhsL <= lhsR);
    }

    // Une famille avant l'autre
    for (int f = 0; f < data.num_circuits; f++) {
        for (int g = 0; g < data.num_circuits; g++) {
            if (f != g) {
                model.addConstr(y[f][g] + y[g][f] == 1);
            }
        }
    }

    // Cohérence sur l'ordre des familles
    for (int f = 0; f < data.num_circuits; f++) {
        for (int g = 0; g < data.num_circuits; g++) {
            if (f != g) {
                for (int h = 0; h < data.num_circuits; h++) {
                    if (h != f && h != g) {
                        model.addConstr(y[f][g] + y[g][h] - 1 <= y[f][h]);
                    }
                }
            }
        }
    }

    // Cohérence sur l'ordre des produits par famille
    for (int f = 0; f < data.num_circuits; f++) {
        for (int g = 0; g < data.num_circuits; g++) {
            if (f != g) {
                for (int j = 0; j < data.num_products; j++) {
                    if (data.product_circuit[j] = f) {
                        for (int jj = 0; jj < data.num_products; jj++) {
                            if (data.product_circuit[jj] = g) {
                                for (int i = 1; i < data.num_racks - 1; i++) {
                                    for (int ii = 1; ii < i; ii++) {
                                        model.addConstr(2 - y[f][g] - x[i][j] >= x[ii][jj]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // La commande passe par les racks contenant ses produits
    for (int c = 0; c < data.num_orders; c++) {
        for (int ii = 1; ii < data.num_racks - 1; ii++) {
            for (int j : data.orders[c]) {
                GRBLinExpr lhs = 0;
                for (int i = 0; i < ii; i++) {
                    lhs += z[c][i][ii];
                }
                model.addConstr(lhs >= x[ii][j]);
            }
        }
    }

    // Contraintes de flot
    for (int c = 0; c < data.num_orders; c++) {
        GRBLinExpr lhs = 0;
        for (int i = 1; i < data.num_racks - 1; i++) {
            lhs += z[c][0][i];
        }
        model.addConstr(lhs == 1);
    }

    for (int c = 0; c < data.num_orders; c++) {
        GRBLinExpr lhs = 0;
        for (int i = 1; i < data.num_racks - 1; i++) {
            lhs += z[c][i][data.num_racks - 1];
        }
        model.addConstr(lhs == 1);
    }

    for (int c = 0; c < data.num_orders; c++) {
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
}