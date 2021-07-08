#include "max_tree.cpp"

class SamePointTransform {
private:
    std::vector<std::vector<MaxTree>> data;
    const std::vector<std::vector<double>>& graph;

public:
    SamePointTransform(const std::vector<std::vector<double>>& g) : graph(g) {}

    void build(std::vector<std::vector<MaxTree>>&& vector_tree) {
        data = std::move(vector_tree);
    }

    void update(int v, int c) {
        int c1 = 0;
        if (v > 0) {
            c1 = data[v][0].hcv();
        } else {
            c1 = data[1][0].hcu();
        }
        int c2 = c;
        for (int u = 0; u != data.size(); ++u) {
            for (int w = 0; w != u; ++w) {
                if (u == v) {
                    data[u][w].set_hcv(c2);
                }
                if (w == v) {
                    data[u][w].set_hcu(c2);
                }
                data[u][w].set(c1, data[u][w].at(c1) - graph[u][v] - graph[w][v]);
                data[u][w].set(c2, data[u][w].at(c2) + graph[u][v] + graph[w][v]);
            }
        }
    }

    std::vector<MaxTree>& operator[](int key) {
        return data[key];
    }

    const std::vector<MaxTree>& operator[](int key) const {
        return data[key];
    }
};