#include <vector>

#include "Model.hpp"
#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    Model M1(data);
    WarehouseSolution sol = M1.solve();
    sol.print();
}
