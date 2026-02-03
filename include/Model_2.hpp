#pragma once

#include "Model.hpp"
#include "Heur_1.hpp"

#include <unordered_set>
#include <random>

/**
 * @class Model
 * @brief MIP Model to solve the warehouse problem
 */
class Model2 : public Model {
   public:

   void print_circuits ();
    /**
     * @brief Class constructor
     */
    Model2 (const WarehouseInstance& Data, int Num_orders);

    /**
     * @brief Solve the instance
     */
    WarehouseSolution solve2();

   private:
    
    std::vector<std::vector<int>> circuits;
    int num_orders;
    std::vector<std::vector<int>> orders;
    std::vector<int> circuit_sequence;
    std::vector<std::vector<int>> racks_circuits;

    void choose_orders(int seed);
    void calcul_racks_circuits();
    vector<int> assignment_real();
};
