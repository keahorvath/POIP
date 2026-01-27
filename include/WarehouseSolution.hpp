#ifndef WAREHOUSESOLUTION_HPP
#define WAREHOUSESOLUTION_HPP
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "WarehouseInstance.hpp"

/**
 * @struct WarehouseSolution
 */
struct WarehouseSolution {
    const WarehouseInstance data;
    std::vector<int> assignment;

    /**
     * @brief Solution constructor
     */
    WarehouseSolution(const WarehouseInstance data, std::vector<int> assignment);

    /**
     * @brief Writes the solution in the given file in the correct format
     * @param filename The file in which to write the solution
     */
    void write(std::string& filename);

    /**
     * @brief Prints the solution in the terminal
     */
    void print();
};

#endif