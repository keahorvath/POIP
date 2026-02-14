#include "WarehouseSolution.hpp"
#ifndef CHECKER_HPP
#define CHECKER_HPP

struct Checker {
    WarehouseSolution sol;
    Checker(WarehouseInstance data, const std::string& file_path);
    std::vector<int> readSolution(const std::string& file_path);
    bool checkAllProductsAssigned();
    bool checkRackCapacity();
    bool checkAeration();
    bool checkCircuitContiguity();
    int calculateCost();
    bool check();
};

#endif
