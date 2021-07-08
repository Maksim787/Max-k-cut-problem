#include <vector>

void merge(
    int* res,
    const std::vector<double>& data,
    int v,
    const int* l = nullptr,
    const int* r = nullptr
) {
    if (!l) {
        if (!r) {
            res[0] = v;
            return;
        } else {
            int i = 0;
            int j = 0;
            while (i + j != 3) {
                if (j == 1) {
                    res[i + j] = r[i];
                    ++i;
                } else {
                    if (r[i] == INT32_MIN) {
                        res[i + j] = v;
                        ++j;
                    } else if (data[v] >= data[r[i]]) {
                        res[i + j] = v;
                        ++j;
                    } else {
                        res[i + j] = r[i];
                        ++i;
                    }
                }
            }
            return;
        }
    } else {
        if (!r) {
            int i = 0;
            int j = 0;
            while (i + j != 3) {
                if (j == 1) {
                    res[i + j] = l[i];
                    ++i;
                } else {
                    if (l[i] == INT32_MIN) {
                        res[i + j] = v;
                        ++j;
                    } else if (data[v] >= data[l[i]]) {
                        res[i + j] = v;
                        ++j;
                    } else {
                        res[i + j] = l[i];
                        ++i;
                    }
                }
            }
            return;
        } else {
            int i = 0;
            int j = 0;
            int k = 0;
            while (i + j + k != 3) {
                if (k == 1) {
                    if (l[i] == INT32_MIN) {
                        res[i + j + k] = r[j];
                        ++j;
                    } else if (r[j] == INT32_MIN) {
                        res[i + j + k] = l[i];
                        ++i;
                    } else if (data[l[i]] >= data[r[j]]) {
                        res[i + j + k] = l[i];
                        ++i;
                    } else {
                        res[i + j + k] = r[j];
                        ++j;
                    }
                } else {
                    if (r[j] == l[i] && l[i] == INT32_MIN) {
                        res[i + j + k] = v;
                        ++k;
                    } else if (r[j] == INT32_MIN) {
                        if (data[v] >= data[l[i]]) {
                            res[i + j + k] = v;
                            ++k;
                        } else {
                            res[i + j + k] = l[i];
                            ++i;
                        }
                    } else if (l[i] == INT32_MIN) {
                        if (data[v] >= data[r[j]]) {
                            res[i + j + k] = v;
                            ++k;
                        } else {
                            res[i + j + k] = r[j];
                            ++j;
                        }
                    } else if (v >= data[r[j]] && v >= data[l[i]]) {
                        res[i + j + k] = v;
                        ++k;
                    } else {
                        if (data[l[i]] >= data[r[j]]) {
                            res[i + j + k] = l[i];
                            ++i;
                        } else {
                            res[i + j + k] = r[j];
                            ++j;
                        }
                    }
                }
            }
            return;
        }
    }
}

class ThreeMaxTree {
public:
    std::vector<double> data;
    int home_coalition;
    std::vector<int[3]> mark;

public:
    ThreeMaxTree() {}

    void build(std::vector<double>&& v, int new_hc) {
        home_coalition = new_hc;
        data = std::move(v);
        mark = std::vector<int[3]>(data.size() + 1);
        for (int i = data.size(); i != 0; --i) {
            mark[i][0] = mark[i][1] = mark[i][2] = -2e9;
            int v = (i - 1 == home_coalition ? -2e9 : i - 1);
            if (2 * i + 1 < mark.size()) {
                merge(mark[i], data, v, mark[2 * i], mark[2 * i + 1]);
            } else if (2 * i < mark.size()) { 
                merge(mark[i], data, v, mark[2 * i]);
            } else {
                merge(mark[i], data, v);
            }
        }
    }

    double at(int key) const {
        return data[key];
    }

    double operator[](int key) const {
        return data[key];
    }

    void set(int key, double value) {
        data[key] = value;
        while (key > 0) {
            mark[key][0] = mark[key][1] = mark[key][2] = -2e9;
            int v = (key - 1 == home_coalition ? -2e9 : key - 1);
            if (2 * key + 1 < mark.size()) {
                merge(mark[key], data, v, mark[2 * key], mark[2 * key + 1]);
            } else if (2 * key < mark.size()) {
                merge(mark[key], data, v, mark[2 * key]);
            } else {
                merge(mark[key], data, v);
            }
            key /= 2;
        }
    }

    int hc() const {                      // Возвращает коалицию игрока
        return home_coalition;
    }

    void set_hc(int new_hc) {       // Меняет коалицию игрока на данную
        home_coalition = new_hc;
    }

    const int* max() {
        return mark[1];
    }
};
