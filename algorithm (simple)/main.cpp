#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <deque>
#include <unordered_set>
#include <utility>
#include <vector>

//#define MOVE_OUTPUT
//#define OPERATION_OUTPUT

// максимизируем сумму рёбер в коалициях (минимизируем разрез)
class Solution {
public:
    int n, k;
    // p - вероятность сделать o3
    // 1 - p - вероятность сделать o4
    double p;
    // omega - максимальное число подряд идущих ходов o3 и o4
    int omega;
    // xi - если проходит столько ходов и мы не улучшаем результат, то делаем o5
    int xi;
    // gamma - число применений o5 подряд
    int gamma;
    double win = 0;
    double best_win = 0;
    int not_improved_cnt = 0;
    int time = 0;
    std::vector<std::vector<double>> w;
    std::vector<std::vector<double>> coalition_interaction; // arr[i][j] сумма рёбер из i игрока в j коалицию
    std::vector<int> person_coalition; // arr[i] - коалиция i игрока
    std::vector<int> person_coalition_answer;
    std::unordered_set<int> tabu_set;
    std::deque<std::pair<int, int>> tabu_list; // (игрок, время окончания бана), снимаем бан, если time > время окончания бана

    Solution(std::vector<std::vector<double>> input_w, int n, int k, double p, int omega, int xi, int gamma) :
            n(n), k(k), p(p), omega(omega), xi(xi), gamma(gamma),
            w(std::move(input_w)),
            coalition_interaction(n, std::vector<double>(n)),
            person_coalition(n),
            person_coalition_answer(n) {
        // все в 0 коалиции
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                coalition_interaction[i][0] += w[i][j];
                win += w[i][j];
            }
        }
        win /= 2;
        best_win = win;
    }

    double random() {
        return static_cast<double>(rand() % 100000) / 100000;
    }

    void run(int n_iter) {
        for (int i = 0; i < n_iter; ++i) {
            std::cout << i << " ИТЕРАЦИЯ" << std::endl;
            double inc_win;
            // пока улучшается делаем o1
            while (true) {
                inc_win = o1();
                if (inc_win <= 0) {
                    break;
                }
                win += inc_win;
            }
            // пока улучшается делаем o2
            while (true) {
                inc_win = o2();
                if (inc_win <= 0) {
                    break;
                }
                win += inc_win;
            }
            // если улучшили, обнуляем счётчик времени не улучшения
            if (win > best_win) {
                best_win = win;
                person_coalition_answer = person_coalition;
                not_improved_cnt = 0;
            } else {
                ++not_improved_cnt;
            }
            if (i == n_iter - 1) {
                return;
            }
            double last_local_optimum = win;
            int move_cnt = 0;
            tabu_set.clear();
            tabu_list.clear();
            // делаем o3 и o4 не более omega раз или пока не найдём лучший результат
            while (move_cnt < omega && win <= last_local_optimum) {
                if (random() < p) {
                    win += o3();
                } else {
                    win += o4();
                }
                ++move_cnt;
            }
            if (not_improved_cnt >= xi) {
                for (int i = 0; i < gamma; ++i) {
                    win += o5();
                }
                not_improved_cnt = xi;
            }
        }
    }

    void print_coalitions() {
        std::cout << "n = " << n << ", " << "k = " << k << "\n";
        for (int person = 0; person < n; ++person) {
            std::cout << person << ": " << person_coalition_answer[person] << "\n";
        }
        std::cout << "best_win = " << best_win << "\n";
    }

    // операция o1
    // ищет человека с наибольшим выигрышем от перемещения в другую коалицию
    // возвращает увеличение выигрыша
    double o1() {
        double max_win = INT32_MIN;
        int max_person, max_coalition;
        for (int person = 0; person < n; ++person) {
            for (int to_coalition = 0; to_coalition < k; ++to_coalition) {
                int from_coalition = person_coalition[person];
                double new_win =
                        coalition_interaction[person][to_coalition] - coalition_interaction[person][from_coalition];
                if (new_win > max_win) {
                    max_win = new_win;
                    max_person = person;
                    max_coalition = to_coalition;
                }
            }
        }
        if (max_win <= 0) {
            return max_win;
        }
#ifdef OPERATION_OUTPUT
        std::cout << "o1" << std::endl;
#endif
        move(max_person, max_coalition);
        return max_win;
    }

    // операция o2
    // ищет двух людей с наибольшим выигрышем от перемещения в другие коалиции
    // возвращает увеличение выигрыша
    double o2() {
        int max_first_person, max_second_person;
        int max_to_first, max_to_second;
        double max_win = INT32_MIN;
        for (int first_person = 0; first_person < n; ++first_person) {
            int from_first = person_coalition[first_person];
            for (int second_person = 0; second_person < n; ++second_person) {
                if (first_person == second_person) {
                    continue;
                }
                for (int to_first = 0; to_first < k; ++to_first) {
                    if (to_first == from_first) {
                        continue;
                    }
                    for (int to_second = 0; to_second < k; ++to_second) {
                        int from_second = person_coalition[second_person];
                        if (to_second == from_second) {
                            continue;
                        }
                        double new_win =
                                coalition_interaction[first_person][to_first] -
                                coalition_interaction[first_person][from_first] +
                                coalition_interaction[second_person][to_second] -
                                coalition_interaction[second_person][from_second] +
                                multiplier(from_first, from_second, to_first, to_second) *
                                w[first_person][second_person];
                        if (new_win > max_win) {
                            max_win = new_win;
                            max_first_person = first_person;
                            max_second_person = second_person;
                            max_to_first = to_first;
                            max_to_second = to_second;
                        }
                    }
                }
            }
        }
        if (max_win <= 0) {
            return max_win;
        }
#ifdef OPERATION_OUTPUT
        std::cout << "o2" << std::endl;
#endif
        move(max_first_person, max_to_first);
        move(max_second_person, max_to_second);
        return max_win;
    }

    double o3() {
        ++time;
        // окончание табу
        while (!tabu_list.empty() && tabu_list.front().second < time) {
            tabu_set.erase(tabu_list.front().first);
            tabu_list.pop_front();
        }
        if (static_cast<int>(tabu_set.size()) == n) {
            return 0;
        }
        double max_win = INT32_MIN;
        int max_person, max_coalition;
        for (int person = 0; person < n; ++person) {
            if (tabu_set.contains(person)) {
                continue;
            }
            for (int to_coalition = 0; to_coalition < k; ++to_coalition) {
                int from_coalition = person_coalition[person];
                double new_win =
                        coalition_interaction[person][to_coalition] - coalition_interaction[person][from_coalition];
                if (new_win > max_win) {
                    max_win = new_win;
                    max_person = person;
                    max_coalition = to_coalition;
                }
            }
        }
        // добавляем человека в табу
        tabu_set.insert(max_person);
        int left = 3;
        int right = n / 10;
        if (left > right) {
            std::swap(left, right);
        }
        int tabu_time;
        if (left == right) {
            tabu_time = 3;
        } else {
            tabu_time = 3 + rand() % (right - left);
        }
        tabu_list.emplace_back(max_person, time + tabu_time);
#ifdef OPERATION_OUTPUT
        std::cout << "o3" << std::endl;
#endif
        move(max_person, max_coalition);
        return max_win;
    }

    double o4() {
        int max_first_person, max_second_person;
        int to_first, to_second;
        double max_win = INT32_MIN;
        do {
            // случайные коалиции, в которые пойдут игроки
            to_first = rand() % k;
            to_second = rand() % k;
            for (int first_person = 0; first_person < n; ++first_person) {
                int from_first = person_coalition[first_person];
                for (int second_person = 0; second_person < n; ++second_person) {
                    if (first_person == second_person) {
                        continue;
                    }
                    if (to_first == from_first) {
                        continue;
                    }
                    int from_second = person_coalition[second_person];
                    if (to_second == from_second) {
                        continue;
                    }
                    double new_win =
                            coalition_interaction[first_person][to_first] -
                            coalition_interaction[first_person][from_first] +
                            coalition_interaction[second_person][to_second] -
                            coalition_interaction[second_person][from_second] +
                            multiplier(from_first, from_second, to_first, to_second) *
                            w[first_person][second_person];
                    if (new_win > max_win) {
                        max_win = new_win;
                        max_first_person = first_person;
                        max_second_person = second_person;
                    }
                }
            }
        } while (max_win == INT32_MIN);
#ifdef OPERATION_OUTPUT
        std::cout << "o4" << std::endl;
#endif
        move(max_first_person, to_first);
        move(max_second_person, to_second);
        return max_win;
    }

    double o5() {
        int person = rand() % n;
        int to_coalition = rand() % k;
        int from_coalition = person_coalition[person];
        double new_win =
                coalition_interaction[person][to_coalition] -
                coalition_interaction[person][from_coalition];
#ifdef OPERATION_OUTPUT
        std::cout << "o5" << std::endl;
#endif
        move(person, to_coalition);
        return new_win;
    }

    int multiplier(int from_first, int from_second, int to_first, int to_second) {
        // from_first -> to_first
        // from_second -> to_second

        if (from_first == from_second &&
            to_first == to_second) {
            return 2;
        }
        if (from_first == to_second &&
            from_second == to_first) {
            return -2;
        }
        if (from_first == from_second &&
            to_first != to_second) {
            return 1;
        }
        if (from_first == to_second &&
            to_first != from_second) {
            return -1;
        }
        if (from_first != from_second &&
            to_first == to_second) {
            return 1;
        }
        if (to_first == from_second &&
            from_first != to_second) {
            return -1;
        }
        if (from_first != from_second &&
            from_first != to_second &&
            to_first != from_second &&
            to_first != to_second) {
            return 0;
        }
        std::cout << "ОШИБКА!" << std::endl;
        std::cout << "from_first = " << from_first << std::endl;
        std::cout << "from_second = " << from_second << std::endl;
        std::cout << "to_first = " << to_first << std::endl;
        std::cout << "to_second = " << to_second << std::endl;
        return INT32_MAX;
    }

    void move(int moved_person, int to_coalition) {
#ifdef MOVE_OUTPUT
        std::cout << "move:\nmoved_person = " << moved_person << "\n";
        std::cout << "to_coalition = " << to_coalition << "\n" << std::endl;
#endif
        int from_coalition = person_coalition[moved_person];
        person_coalition[moved_person] = to_coalition;
        for (int person = 0; person < n; ++person) {
            coalition_interaction[person][from_coalition] -= w[person][moved_person];
            coalition_interaction[person][to_coalition] += w[person][moved_person];
        }
    }
};

int main() {
    srand(1);
    std::fstream file("../input.txt");
    int n, k;
    file >> n >> k;
    std::vector<std::vector<double>> w(n, std::vector<double>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file >> w[i][j];
        }
    }
//    ω = 500, ξ = 1000, ρ = 0.5, γ = 0.1|V|,
    double p = 0.5;
    int omega = 500;
    int xi = 10;
    int gamma = std::max(1, static_cast<int>(0.1 * n));
    Solution s(w, n, k, p, omega, xi, gamma);
    int n_iter = 100;
    s.run(n_iter);
    s.print_coalitions();

    return 0;
}
