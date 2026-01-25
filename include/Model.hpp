#pragma once

#include "WarehouseInstance.hpp"

#include <iostream>
#include <string>
#include <fstream>

class Modele {

    public :

        Modele(const WarehouseInstance &Data);

        void solve ();

        void write_sol(std::string &filename);

        void print_sol();

    private :

        WarehouseInstance data;

        std::vector<int> assignment;
};
