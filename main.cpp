#include <vector>

#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"

using namespace std;

int main(int argc, char** argv) {
    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();
}
