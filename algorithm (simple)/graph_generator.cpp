#include <cstdlib>
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
    std::cout << n << " " << k << "\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << graph[i][j] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
