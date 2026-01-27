#include "WarehouseSolution.hpp"
using namespace std;

WarehouseSolution::WarehouseSolution(const WarehouseInstance data_, std::vector<int> assignment_) : data(data_), assignment(assignment_) {};

void WarehouseSolution::write(string& filename) {
    ofstream file(filename);

    file << data.num_products << endl;

    for (int i : assignment) {
        file << i << endl;
    }
}

void WarehouseSolution::print() {
    for (int j = 0; j < data.num_products; j++) {
        cout << "Product " << j << " : Rack " << assignment[j] << endl;
    }
}