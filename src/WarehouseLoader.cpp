#include "WarehouseLoader.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

WarehouseLoader::WarehouseLoader(string warehouse_dir_) : warehouse_dir(warehouse_dir_) {};

vector<vector<int>> WarehouseLoader::loadAdjacencyMatrix() {
    string file_name = warehouse_dir + "/rack_adjacency_matrix.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    int nb_racks;
    file >> nb_racks;
    vector<vector<int>> adj_matrix(nb_racks);
    for (int i = 0; i < nb_racks; i++) {
        for (int j = 0; j < nb_racks; j++) {
            int val;
            file >> val;
            adj_matrix[i].push_back(val);
        }
    }
    return adj_matrix;
}

vector<int> WarehouseLoader::loadRackCapacity() {
    string file_name = warehouse_dir + "/rack_capacity.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    int nb_racks;
    file >> nb_racks;
    vector<int> rack_cap;
    for (int i = 0; i < nb_racks; i++) {
        int val;
        file >> val;
        rack_cap.push_back(val);
    }
    return rack_cap;
}

vector<int> WarehouseLoader::loadProductCircuits() {
    string file_name = warehouse_dir + "/product_circuit.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    int nb_prod;
    file >> nb_prod;
    vector<int> prod_circuits;
    for (int i = 0; i < nb_prod; i++) {
        int val;
        file >> val;
        prod_circuits.push_back(val);
    }
    return prod_circuits;
}

vector<vector<int>> WarehouseLoader::loadAislesRacks() {
    string file_name = warehouse_dir + "/aisle_racks.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    int nb_aisles;
    file >> nb_aisles;
    vector<vector<int>> aislesracks(nb_aisles);
    for (int i = 0; i < nb_aisles; i++) {
        int nb_racks;
        file >> nb_racks;
        for (int j = 0; j < nb_racks; j++) {
            int val;
            file >> val;
            aislesracks[i].push_back(val);
        }
    }
    return aislesracks;
}

vector<vector<int>> WarehouseLoader::loadOrders() {
    string file_name = warehouse_dir + "/orders.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    int nb_orders;
    file >> nb_orders;
    vector<vector<int>> orders(nb_orders);
    for (int i = 0; i < nb_orders; i++) {
        int nb_prods;
        file >> nb_prods;
        for (int j = 0; j < nb_prods; j++) {
            int val;
            file >> val;
            orders[i].push_back(val);
        }
    }
    return orders;
}

vector<int> WarehouseLoader::loadMetadata() {
    string file_name = warehouse_dir + "/metadata.txt";
    ifstream file(file_name);
    if (!file) {
        throw runtime_error("Error: cannot open file " + file_name);
    }
    vector<int> metadata;
    for (int i = 0; i < 7; i++) {
        int val;
        file >> val;
        metadata.push_back(val);
    }
    return metadata;
}

WarehouseInstance WarehouseLoader::loadAll() {
    WarehouseInstance inst;

    inst.adjacency = loadAdjacencyMatrix();
    inst.rack_capacity = loadRackCapacity();
    inst.product_circuit = loadProductCircuits();
    inst.aisles_racks = loadAislesRacks();
    inst.orders = loadOrders();

    vector<int> metadata = loadMetadata();

    inst.num_racks = metadata[0];
    inst.total_slots = metadata[1];
    inst.aeration_rate = metadata[2];
    inst.num_products = metadata[3];
    inst.num_circuits = metadata[4];
    inst.num_aisles = metadata[5];
    inst.num_orders = metadata[6];

    return inst;
}