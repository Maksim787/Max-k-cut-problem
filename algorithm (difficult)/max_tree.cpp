#include <vector>

class MaxTree {
public:
    std::vector<double> data;
    int home_coalition_v;
    int home_coalition_u;
    std::vector<int> mark;

public:
    MaxTree() {}

    void build(std::vector<double>&& v, int new_hcv, int new_hcu) {
        data = std::move(v);
        home_coalition_v = new_hcv;
        home_coalition_u = new_hcu;
        mark.resize(data.size() + 1);
        for (int i = data.size(); i != 0; --i) {
            mark[i] = INT32_MIN;
            if (i - 1 != home_coalition_v && i - 1 != home_coalition_u) {
                mark[i] = i - 1;
            }
            if (2 * i <= data.size()) {
                if (mark[2 * i] != INT32_MIN) {
                    if (mark[i] == INT32_MIN) {
                        mark[i] = mark[2 * i];
                    } else if (data[mark[2 * i]] > data[mark[i]]) {
                        mark[i] = mark[2 * i];
                    }
                }
            }
            if (2 * i + 1 <= data.size()) {
                if (mark[2 * i + 1] != INT32_MIN) {
                    if (mark[i] == INT32_MIN) {
                        mark[i] = mark[2 * i + 1];
                    } else if (data[mark[2 * i + 1]] > data[mark[i]]) {
                        mark[i] = mark[2 * i + 1];
                    }
                }
            }
        }
    }

    double at(int key) {
        return data[key];
    }

    void set(int key, double value) {
        data[key] = value;
        while (key > 0) {
            mark[key] = INT32_MIN;
            if (key - 1 != home_coalition_v && key - 1 != home_coalition_u) {
                mark[key] = key - 1;
            }
            if (2 * key <= data.size()) {
                if (mark[2 * key] != INT32_MIN) {
                    if (mark[key] == INT32_MIN) {
                        mark[key] = mark[2 * key];
                    } else if (data[mark[2 * key]] > data[mark[key]]) {
                        mark[key] = mark[2 * key];
                    }
                }
            }
            if (2 * key + 1 <= data.size()) {
                if (mark[2 * key + 1] != INT32_MIN) {
                    if (mark[key] == INT32_MIN) {
                        mark[key] = mark[2 * key + 1];
                    } else if (data[mark[2 * key + 1]] > data[mark[key]]) {
                        mark[key] = mark[2 * key + 1];
                    }
                }
            }
            key /= 2;
        }
    }

    int hcv() {                     // Возвращает коалицию первого игрока
        return home_coalition_v;
    }

    int hcu() {                     // Возвращает коалицию второго игрока
        return home_coalition_u;
    }

    void set_hcv(int new_hcv) {     // Меняет коалицию первого игрока на данную
        home_coalition_v = new_hcv;
    }

    void set_hcu(int new_hcu) {     // Меняет коалицию второго игрока на данную
        home_coalition_u = new_hcu;
    }

    int max() {
        return mark[1];
    }
};
