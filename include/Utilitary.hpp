#ifndef UTILITARY_HPP
#define UTILITARY_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "WarehouseInstance.hpp"
#include "WarehouseSolution.hpp"

// Contains all the utilitary functions used in the project

std::ifstream openFile(const std::string& filename);

void swap(int& a, int& b);
void swap(std::vector<int>& a, std::vector<int>& b);

int calculateCost(const WarehouseSolution& solution);

// File parsing functions for heuristics

/**
 * @brief Reads a file containing circuit frequencies and returns a vector of circuit indices sorted in descending order of frequency.
 */
std::vector<int> readFrequencyCircuits(const std::string& filename, int num_circuits);

/**
 * @brief Reads a file containing product frequencies and returns a vector associating each circuit with the products it contains, sorted in
 * descending order of frequency.
 */
std::vector<std::vector<int>> readFreqProd(std::string file_name, int num_circuit, std::vector<int> product_circuit);

/**
 * @brief Returns a vector associating each circuit with the products that have never been used in any order, based on a frequency file.
 */
std::vector<std::vector<int>> readNeverUsedProducts(std::string file_name, int num_circuit, int num_products, std::vector<int> product_circuit);

/**
 * @brief Reads a file containing pairs of products that are frequently ordered together and returns an unordered_map associating each product with
 * the list of products it is frequently paired with.
 */
std::unordered_map<int, std::vector<int>> readProductPairs(const std::string& file_name, int num_products);

std::vector<std::vector<int>> readConcordanceCircuit(const std::string& file_name);

void readFrequencyCircuitsAndCounts(const std::string& file_name, int num_circuits, std::vector<int>& frequency_circuits,
                                    std::vector<int>& freq_count);

// Other utilitary functions

std::vector<int> buildCircuitOrderByConcordanceFastWeighted(const std::vector<int>& frequency_circuits, const std::vector<std::vector<int>>& concord,
                                                            const std::vector<int>& freq_count);
/**
 * @brief Associates each circuit with the set of products it contains, based on the product_circuit vector in the WarehouseInstance.
 */
std::vector<std::vector<int>> buildProductInCircuit(const WarehouseInstance& data);

/**
 * @brief Calculates the number of free locations needed for aeration in each aisle, based on the total capacity of the racks in the aisle and the
 */
std::vector<int> nbFreeLoc(const WarehouseInstance& data);

/**
 * @brief Modify the capacity of racks by adding aeration in the rightmost racks of each aisle
 */
std::vector<int> newEndRackCapacity(const WarehouseInstance& data);

/**
 * @brief Modify the capacity of racks by adding aeration randomly in each aisle.
 */
std::vector<int> newRandomRackCapacity(const WarehouseInstance& data);

/**
 * @brief Calculates the factorial of n, capped at a maximum threshold.
 */
long long int factorial(int n, long long cap);
#endif