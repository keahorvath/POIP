#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"
#include "Model.hpp"
#include "Model_2.hpp"
#include "Heur_1.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    Model2 M(data);
    M.print_circuits();
    M.solve2();

    return 0;
}
