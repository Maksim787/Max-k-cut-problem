#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
    int n, k;
    std::cin >> n >> k;
    std::vector<std::vector<double>> graph(n, std::vector<double>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            graph[i][j] = (rand() % 11) - 6;
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            graph[i][j] = graph[j][i];
        }
    }
    std::ofstream file("../input.txt");
    file << n << " " << k << "\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << graph[i][j] << " ";
        }
        file << "\n";
    }

    return 0;
}
