#include "gurobi_c++.h"

using namespace std;

class WarehouseInstance {

    public:

    vector<vector<int>> adjacency;
    vector<int> rank_capacity;
    vector<int> product_circuit;
    vector<vector<int>> aisles_racks;
    vector<int> orders;

    int num_racks;
    int total_slots;
    int aeration_rate;
    int num_products;
    int num_circuits;
    int num_aisles;
    int num_orders;

};

int main(int argc, char** argv) {
    cout << "blabla" << endl;

    GRBEnv* env;
    GRBModel* model;
}
