#ifndef WAREHOUSELOADER_HPP
#define WAREHOUSELOADER_HPP

#include <string>

#include "WarehouseInstance.hpp"

struct WarehouseLoader {
    std::string warehouse_dir;

    WarehouseLoader(std::string warehouse_dir);
    std::vector<std::vector<int>> loadAdjacencyMatrix();
    std::vector<int> loadRackCapacity();
    std::vector<int> loadProductCircuits();
    std::vector<std::vector<int>> loadAislesRacks();
    std::vector<std::vector<int>> loadOrders();
    std::vector<int> loadMetadata();
    WarehouseInstance loadAll();
};
#endif