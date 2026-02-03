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
   void print_racks_circuits();
    /**
     * @brief Class constructor
     */
    Model2 (const WarehouseInstance& Data, int Num_orders, vector<int> Circuit_sequence);

    /**
     * @brief Solve the instance
     */
    WarehouseSolution solve2();

   private:
    
    std::vector<std::vector<int>> circuits; // Each element is a circuit, composed of products
    int num_orders;
    std::vector<std::vector<int>> orders; // Each element is an order, composed of products
    std::vector<int> circuit_sequence; // Sequence of circuits : vector of size "number of circuits"
    std::vector<std::vector<int>> racks_circuits; // Beginning rack and end rack of each circuit, given the sequence. Aeration is at the end of each aisle

    void choose_orders(int seed);
    void calcul_racks_circuits();
};
