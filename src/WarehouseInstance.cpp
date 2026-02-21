#include "WarehouseInstance.hpp"

using namespace std;

void WarehouseInstance::print() {
    cout << num_racks << " racks" << endl;
    cout << total_slots << " total locations" << endl;
    cout << "Aeration rate : " << aeration_rate << "%" << endl;
    cout << num_products << " products" << endl;
    cout << num_circuits << " circuits" << endl;
    cout << num_aisles << " aisles" << endl;
    cout << num_orders << " orders" << endl;

    cout << endl;
    for (int i = 0; i < num_racks; i++) {
        cout << "Rack " << i << " : Capacity " << rack_capacity[i] << endl;
    }

    cout << endl;
    for (int j = 0; j < num_products; j++) {
        cout << "Product " << j << " : Circuit " << product_circuit[j] << endl;
    }

    cout << endl;
    for (int a = 0; a < num_aisles; a++) {
        cout << "Aisle " << a << " : Racks " << aisles_racks[a][0] << " to " << aisles_racks[a][aisles_racks[a].size() - 1] << endl;
    }

    cout << endl;
    for (int c = 0; c < num_orders; c++) {
        cout << "Order " << c << " : Products ";
        for (int p : orders[c]) {
            cout << p << ", ";
        }
        cout << endl;
    }
    cout << endl;
}
