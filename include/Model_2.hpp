#pragma once

#include "Model.hpp"

/**
 * @class Model
 * @brief MIP Model to solve the warehouse problem
 */
class Model2 : public Model {
   public:

   //std::vector<std::vector<int>> circuits;

   void print_circuits ();
    /**
     * @brief Class constructor
     */
    Model2 (const WarehouseInstance& Data);

    /**
     * @brief Solve the instance
     */
    WarehouseSolution solve2();

   private:
    //const WarehouseInstance data;
    std::vector<std::vector<int>> circuits;
};
