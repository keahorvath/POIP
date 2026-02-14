#include <vector>

#include "Model.hpp"
#include "WarehouseInstance.hpp"
#include "WarehouseLoader.hpp"

using namespace std;

int main(int argc, char** argv) {

    WarehouseLoader loader("../warehouse_toy");
    WarehouseInstance data = loader.loadAll();

    Model M1(data);
    WarehouseSolution sol = M1.solve();
    sol.print();
    string fsol = "../solutions/toy.txt";
    sol.write(fsol);

    Checker checker(data, "../solutions/toy.txt");
    bool check = checker.check();
    if (check) {
        cout << "Solution is valid" << endl;
        cout << "COST : " << checker.calculateCost() << endl;
    } else {
        cout << " Solution is not valid" << endl;
    }
    return 0;
    return 0;
}


