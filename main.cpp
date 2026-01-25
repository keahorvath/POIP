#include <vector>

#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"
#include "Model.hpp"

using namespace std;

int main(int argc, char** argv) {
    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    Modele M1(data);
    M1.solve();
    M1.print_sol();

}
