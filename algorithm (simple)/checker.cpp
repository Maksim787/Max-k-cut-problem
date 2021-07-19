#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::ifstream output("../output.txt");
    std::ifstream input("../input.txt");
    int n, k;
    input >> n >> k;
    std::vector<std::vector<double>> w(n, std::vector<double>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            input >> w[i][j];
        }
    }
    std::vector<int> person_coalition(n);
    for (int i = 0; i < n; ++i) {
        output >> person_coalition[i];
    }
    double win = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (person_coalition[i] == person_coalition[j]) {
                win += w[i][j];
            }
        }
    }
    std::cout << "win: " << win << "\n";
    return 0;
}
