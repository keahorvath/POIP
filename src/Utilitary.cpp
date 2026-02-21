#include "Utilitary.hpp"

#include <cmath>

using namespace std;
ifstream openFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Error: cannot open file " + filename);
    }
    return file;
}

void swap(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

void swap(vector<int>& a, vector<int>& b) {
    vector<int> tmp = a;
    a = b;
    b = tmp;
}

int calculateCost(const WarehouseSolution& solution) {
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
        vector<int> path;
        path.push_back(start);
        path.insert(path.end(), racks_set.begin(), racks_set.end());
        path.push_back(end);
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            total += adj[path[i]][path[i + 1]];
        }
    }
    return total;
}

vector<int> readFrequencyCircuits(string filename, int num_circuits) {
    vector<int> frequency_circuits;
    ifstream file = openFile(filename);
    string line;
    getline(file, line);  // skip header
    // Read to end of file
    while (getline(file, line)) {
        int id_circuit;
        int nb_prod_in_family;
        int count;
        double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_circuit >> nb_prod_in_family >> count >> pourcentage)) {
            break;
        }  // error
        frequency_circuits.push_back(id_circuit);
    }
    file.close();

    // Add all circuits that do not appear in the file (frequency 0):
    for (int c = 0; c < num_circuits; c++) {
        if (find(frequency_circuits.begin(), frequency_circuits.end(), c) == frequency_circuits.end()) {
            frequency_circuits.push_back(c);
        }
    }
    return frequency_circuits;
}

vector<vector<int>> readFreqProd(string file_name, int num_circuit, vector<int> product_circuit) {
    vector<vector<int>> freq_prod(num_circuit);
    ifstream file = openFile(file_name);
    string line;
    getline(file, line);  // skip header
    // Read to end of file
    while (getline(file, line)) {
        int id_prod;
        int count;
        double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_prod >> count >> pourcentage)) {
            break;
        }  // error
        freq_prod[product_circuit[id_prod]].push_back(id_prod);
    }
    file.close();
    return freq_prod;
}

vector<vector<int>> readNeverUsedProducts(string file_name, int num_circuit, int num_products, vector<int> product_circuit) {
    set<int> used;
    vector<vector<int>> never_used(num_circuit);
    ifstream file = openFile(file_name);
    string line;
    getline(file, line);  // skip header
    // Read to end of file
    while (getline(file, line)) {
        int id_prod;
        int count;
        double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_prod >> count >> pourcentage)) {
            break;
        }  // error
        used.insert(id_prod);
    }
    file.close();
    int count = 0;
    for (int p = 0; p < num_products; p++) {
        if (used.find(p) == used.end()) {
            int c_id = product_circuit[p];          // Get circuit ID
            if (c_id >= 0 && c_id < num_circuit) {  // Security
                never_used[c_id].push_back(p);
                count++;
            }
        }
    }
    cout << "Number of never used products ratio : " << count * 100.0 / num_products << " % and count : " << count << endl;
    return never_used;
}

unordered_map<int, vector<int>> readProductPairs(const string& file_name, int num_products) {
    unordered_map<int, vector<int>> pairs;
    ifstream file = openFile(file_name);
    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;
        istringstream iss(line);
        int p;
        iss >> p;
        if (p < 0 || p >= num_products) continue;
        vector<int> neigh;
        int q;
        while (iss >> q) {
            if (q == -1) break;  // No neighbors
            if (q < 0 || q >= num_products) continue;
            if (q == p) continue;
            neigh.push_back(q);
        }
        pairs[p] = neigh;  // Empty if -1 or nothing
    }
    file.close();
    // Make sure every product is in the map, even if it has no pairs
    for (int p = 0; p < num_products; p++) {
        if (pairs.find(p) == pairs.end()) {
            pairs[p] = vector<int>();
        }
    }
    return pairs;
}

vector<vector<int>> readConcordanceCircuit(const string& file_name) {
    ifstream file = openFile(file_name);
    int n;
    file >> n;
    vector<vector<int>> concord(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> concord[i][j];
        }
    }
    file.close();
    return concord;
}

void readFrequencyCircuitsAndCounts(const string& file_name, int num_circuits, vector<int>& frequency_circuits, vector<int>& freq_count) {
    frequency_circuits.clear();
    freq_count.assign(num_circuits, 0);
    ifstream file = openFile(file_name);
    string line;
    getline(file, line);  // skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        int id_circuit;
        int nb_prod_in_family;
        int count;
        double pourcentage;
        istringstream iss(line);
        if (!(iss >> id_circuit >> nb_prod_in_family >> count >> pourcentage)) break;
        if (id_circuit < 0 || id_circuit >= num_circuits) continue;
        frequency_circuits.push_back(id_circuit);
        freq_count[id_circuit] = count;
    }
    file.close();
    // Add all circuits that do not appear in the file (frequency 0):
    for (int c = 0; c < num_circuits; c++) {
        if (find(frequency_circuits.begin(), frequency_circuits.end(), c) == frequency_circuits.end()) {
            frequency_circuits.push_back(c);
        }
    }
    for (int c = 0; c < num_circuits; c++) {
        if (find(frequency_circuits.begin(), frequency_circuits.end(), c) == frequency_circuits.end()) {
            frequency_circuits.push_back(c);
        }
    }
}

vector<int> buildCircuitOrderByConcordanceFastWeighted(const vector<int>& frequency_circuits, const vector<vector<int>>& concord,
                                                       const vector<int>& freq_count) {
    int n = (int)concord.size();
    vector<int> order;
    order.reserve(n);
    vector<char> used(n, 0);
    // Cumutative score
    vector<int> score(n, 0);
    // 1) first
    int first = frequency_circuits[0];
    order.push_back(first);
    used[first] = 1;
    // init scores
    for (int x = 0; x < n; x++) {
        score[x] = freq_count[first] * concord[first][x];
    }
    // 2) glouton
    while ((int)order.size() < n) {
        int best = -1;
        int best_score = -1;
        for (int x = 0; x < n; x++) {
            if (used[x]) continue;
            if (score[x] > best_score) {
                best_score = score[x];
                best = x;
            }
        }
        if (best == -1) break;
        order.push_back(best);
        used[best] = 1;
        // update
        for (int x = 0; x < n; x++) {
            if (used[x]) continue;
            score[x] += freq_count[best] * concord[best][x];
        }
    }
    return order;
}

vector<vector<int>> buildProductInCircuit(const WarehouseInstance& data) {
    vector<vector<int>> product_in_circuit(data.num_circuits);
    for (int p = 0; p < data.num_products; p++) {
        int c_id = data.product_circuit[p];
        if (c_id >= 0 && c_id < data.num_circuits) {
            product_in_circuit[c_id].push_back(p);
        }
    }
    return product_in_circuit;
}
vector<int> nbFreeLoc(const WarehouseInstance& data) {
    vector<int> num_free_loc(data.num_aisles, 0);
    for (int i = 0; i < data.num_aisles; i++) {
        for (int j = 0; j < (int)data.aisles_racks[i].size(); j++) {
            num_free_loc[i] += data.rack_capacity[data.aisles_racks[i][j]];
        }
        num_free_loc[i] = ceil(num_free_loc[i] * data.aeration_rate / 100.);
    }
    return num_free_loc;
}

vector<int> newEndRackCapacity(const WarehouseInstance& data) {
    vector<int> num_free_loc = nbFreeLoc(data);
    vector<int> new_rack_capacity = data.rack_capacity;
    vector<int> num_racksPerAisle(data.num_aisles);
    for (int i = 0; i < data.num_aisles; i++) {
        num_racksPerAisle[i] = data.aisles_racks[i].size();
    }
    for (int i = 0; i < data.num_aisles; i++) {
        int k = 1;
        while (num_free_loc[i] > data.rack_capacity[data.aisles_racks[i][num_racksPerAisle[i] - k]]) {
            new_rack_capacity[data.aisles_racks[i][num_racksPerAisle[i] - k]] = 0;
            num_free_loc[i] -= data.rack_capacity[data.aisles_racks[i][num_racksPerAisle[i] - k]];
            k++;
        }
        new_rack_capacity[data.aisles_racks[i][num_racksPerAisle[i] - k]] -= num_free_loc[i];
    }
    return new_rack_capacity;
}

vector<int> newRandomRackCapacity(const WarehouseInstance& data) {
    vector<int> rack_capacity = data.rack_capacity;
    unordered_map<int, int> aeration_rack;
    for (int i = 0; i < data.num_aisles; i++) {
        int aisle_capacity = 0;
        for (int rack : data.aisles_racks[i]) {
            aisle_capacity += data.rack_capacity[rack];
        }
        int nb_aeration_rack =
            ceil(aisle_capacity * data.aeration_rate / 100.);  // Returns the smallest integer >= aisle_capacity*data.aeration_rate/100.
        while (nb_aeration_rack > 0) {
            int random_rack_index = rand() % data.aisles_racks[i].size();
            int random_rack = data.aisles_racks[i][random_rack_index];
            if (rack_capacity[random_rack] > 0) {
                rack_capacity[random_rack] -= 1;
                nb_aeration_rack -= 1;
                aeration_rack[random_rack] += 1;
            }
        }
    }
    return rack_capacity;
}

long long int factorial(int n, long long cap) {
    long long res = 1;
    for (int i = 2; i <= n; i++) {
        if (res > cap / i) return cap;
        res *= i;
        if (res >= cap) return cap;
    }
    return res;
}