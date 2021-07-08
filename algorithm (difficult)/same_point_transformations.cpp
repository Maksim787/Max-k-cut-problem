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
                data[u][w].set(c1, data[u][w].at(c1) - graph[u][v] - graph[w][v]);
                data[u][w].set(c2, data[u][w].at(c2) + graph[u][v] + graph[w][v]);
            }
        }
        for (int u = 0; u != v; ++u) {
            auto x = data[v][u].data;
            data[v][u].build(std::move(x), c2, data[v][u].hcu());
        }
        for (int u = v + 1; u != data.size(); ++u) {
            auto x = data[u][v].data;
            data[u][v].build(std::move(x), data[u][v].hcv(), c2);
        }
    }

    std::vector<MaxTree>& operator[](int key) {
        return data[key];
    }

    const std::vector<MaxTree>& operator[](int key) const {
        return data[key];
    }
};