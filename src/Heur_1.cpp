#include "Heur_1.hpp"

using namespace std;

int calculate_cost(const WarehouseSolution& solution) {
    const auto& adj = solution.data.adjacency;
    int start = 0;
    int end = solution.data.num_racks - 1;

    int total = 0;

    for (const auto& order : solution.data.orders) {
        
        // Add racks
        set<int> racks_set;
        for (int product : order) {
            racks_set.insert(solution.assignment[product]);
        }

        // Build path
        std::vector<int> path;
        path.push_back(start);
        path.insert(path.end(), racks_set.begin(), racks_set.end());
        path.push_back(end);

        
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            total += adj[path[i]][path[i + 1]];
        }
    }

    return total;
}


