#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

/**
 * @class Model
 * @brief MIP Model to solve the warehouse problem
 */
class Model {
   public:
    /**
     * @brief Class constructor
     */
    Model(const WarehouseInstance& Data);

    /**
     * @brief Solve the instance
     */
    WarehouseSolution solve();

   private:
    const WarehouseInstance data;
};
