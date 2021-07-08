#include "three_max_tree.cpp"

class PB_Vector {
private: 
    std::vector<ThreeMaxTree> data;
    const std::vector<std::vector<double>>& graph;

public:
    PB_Vector(const std::vector<std::vector<double>>& g) : graph(g) {}

    void build(std::vector<ThreeMaxTree>&& vector_tree) {
        data = std::move(vector_tree);
    }

    void update(int v, int c) {
        int c1 = data[v].hc();
        int c2 = c;
        data[v].set_hc(c2);
        for (int u = 0; u != data.size(); ++u) {
            data[u].set(c1, data[u].at(c1) - graph[u][v]);
            data[u].set(c2, data[u].at(c2) + graph[u][v]);
        }
    }

    ThreeMaxTree& operator[](int key) {
        return data[key];
    }

    const ThreeMaxTree& operator[](int key) const {
        return data[key];
    }
};