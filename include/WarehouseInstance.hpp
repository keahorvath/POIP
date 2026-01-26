#ifndef WAREHOUSEINSTANCE_HPP
#define WAREHOUSEINSTANCE_HPP

#include <vector>
#include <iostream>

struct WarehouseInstance {
    std::vector<std::vector<int>> adjacency;
    std::vector<int> rack_capacity;
    std::vector<int> product_circuit;
    std::vector<std::vector<int>> aisles_racks;
    std::vector<std::vector<int>> orders;

    int num_racks;
    int total_slots;
    int aeration_rate;
    int num_products;
    int num_circuits;
    int num_aisles;
    int num_orders;

    void print();
};

#endif