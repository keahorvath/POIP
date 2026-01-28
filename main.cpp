#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"
#include "Model.hpp"
#include "Heur_1.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    WarehouseSolution sol = Heur_1 (data, 15);

    return 0;
}
