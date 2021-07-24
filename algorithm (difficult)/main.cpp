#include "potential_benefits.cpp"
#include "same_point_transformations.cpp"
#include "o2_update.cpp"
#include "timer.cpp"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <vector>

//#define DEBUG
//#define MOVE_OUTPUT
//#define OPERATION_OUTPUT

#define op1
#define op2
#define op3
#define op4
#define op5

// максимизируем сумму рёбер в коалициях (минимизируем разрез)
class CoalitionApprox {
public:
    const int n, k;
    // p - вероятность сделать o3
    // 1 - p - вероятность сделать o4
    const double p;
    // omega - максимальное число подряд идущих ходов o3 и o4
    const int omega;
    // xi - если проходит столько ходов и мы не улучшаем результат, то делаем o5
    const int xi;
    // gamma - число применений o5 подряд
    const int gamma;
    const double eps = 1e-9;
    double win = 0;
    double best_win = 0;
    int not_improved_cnt = 0;
    int time = 0;
    int o1_cnt = 0, o2_cnt = 0, o3_cnt = 0, o4_cnt = 0, o5_cnt = 0;
    double o1_time = 0, o2_time = 0, o3_time = 0, o4_time = 0, o5_time = 0;
    std::vector<std::vector<double>> w;
    PB_Vector pb_vector;
    SamePointTransform same_point_transform;
    o2Updater o2_updater;
    std::vector<int> person_coalition; // arr[i] - коалиция i игрока
    std::vector<int> person_coalition_answer;
    std::unordered_set<int> tabu_set;
    std::deque<std::pair<int, int>> tabu_list; // (игрок, время окончания бана), снимаем бан, если time > время окончания бана

    CoalitionApprox(std::vector<std::vector<double>> input_w, int n, int k, double p, int omega, int xi, int gamma,
                    bool all_in_one = false) :
            n(n), k(k), p(p), omega(omega), xi(xi), gamma(gamma),
            w(std::move(input_w)),
            pb_vector(w), same_point_transform(w), o2_updater(pb_vector, w),
            person_coalition(n),
            person_coalition_answer(n) {
        // все в 0 коалиции
        if (all_in_one) {
            for (int person = 0; person < n; ++person) {
                person_coalition[person] = 0;
            }
        } else {
            // все в случайных коалициях
            for (int person = 0; person < n; ++person) {
                person_coalition[person] = rand() % k;
            }
        }
        person_coalition_answer = person_coalition;
        // построение pb_vector
        std::vector<ThreeMaxTree> pb_vector_init(n);
        for (int first_person = 0; first_person < n; ++first_person) {
            std::vector<double> three_max_tree_init(k);
            for (int second_person = 0; second_person < n; ++second_person) {
                three_max_tree_init[person_coalition[second_person]] += w[first_person][second_person];
                if (person_coalition[first_person] == person_coalition[second_person] && first_person < second_person) {
                    win += w[first_person][second_person];
                }
            }
            pb_vector_init[first_person].build(std::move(three_max_tree_init), person_coalition[first_person]);
        }
        pb_vector.build(std::move(pb_vector_init));
        best_win = win;

        // построение same_point_transform
        // build_same_point_transform();
#ifdef DEBUG
        double real_win = get_real_win();
        assert(real_win - eps < win);
        assert(real_win + eps > win);
#endif
    }

    void build_same_point_transform() {
        // построение same_point_transform
        std::vector<std::vector<MaxTree>> same_point_transform_init(n);
        for (int first_person = 0; first_person < n; ++first_person) {
            same_point_transform_init[first_person] = std::vector<MaxTree>(first_person);
            for (int second_person = 0; second_person < first_person; ++second_person) {
                std::vector<double> max_tree_init(k);
                for (int coalition = 0; coalition < k; ++coalition) {
                    max_tree_init[coalition] = pb_vector[first_person][coalition] +
                                               pb_vector[second_person][coalition];
                }
                same_point_transform_init[first_person][second_person].build(std::move(max_tree_init), 0, 0);
            }
        }
        same_point_transform.build(std::move(same_point_transform_init));
    }

    static double random() {
        return static_cast<double>(rand()) / RAND_MAX;
    }

    void run(int n_iter) {
        for (int j = 0; j < n_iter; ++j) {
            std::cout << j + 1 << " ITERATION" << std::endl;
            double inc_win;
#ifdef op1
            // пока улучшается делаем o1
            while (true) {
                inc_win = o1();
                if (inc_win <= eps) {
                    break;
                }
                win += inc_win;
#ifdef DEBUG
                double real_win = get_real_win();
                assert(real_win - eps < win);
                assert(real_win + eps > win);
#endif
            }
#endif
#ifdef op2
            // пока улучшается делаем o2
            build_same_point_transform();
            while (true) {
                inc_win = o2();
                if (inc_win <= eps) {
                    break;
                }
                win += inc_win;
#ifdef DEBUG
                double real_win = get_real_win();
                assert(real_win - eps < win);
                assert(real_win + eps > win);
#endif
            }
#endif
            // если улучшили, обнуляем счётчик времени не улучшения
            if (win > best_win) {
                best_win = win;
                person_coalition_answer = person_coalition;
                not_improved_cnt = 0;
            } else {
                ++not_improved_cnt;
            }

            double last_local_optimum = win;
            int move_cnt = 0;
            tabu_set.clear();
            tabu_list.clear();
            // делаем o3 и o4 не более omega раз или пока не найдём лучший результат
#if defined(op3) || defined(op4)
            while (move_cnt < omega && win <= last_local_optimum) {
                if (random() < p) {
#ifdef op3
                    win += o3();
#endif
                } else {
#ifdef op4
                    win += o4();
#endif
                }
#ifdef DEBUG
                double real_win = get_real_win();
                assert(real_win - eps < win);
                assert(real_win + eps > win);
#endif
                ++move_cnt;
            }
#endif
#ifdef op5
            if (not_improved_cnt >= xi) {
                for (int i = 0; i < gamma; ++i) {
                    win += o5();
#ifdef DEBUG
                    double real_win = get_real_win();
                    assert(real_win - eps < win);
                    assert(real_win + eps > win);
#endif
                }
                not_improved_cnt = xi;
            }
#endif
        }
    }

    void print_coalitions() {
        std::ofstream file("../output.txt");
        std::cout << "n = " << n << ", " << "k = " << k << "\n";
        for (int person = 0; person < n; ++person) {
            std::cout << person << ": " << person_coalition_answer[person] << "\n";
            file << person_coalition_answer[person] << " ";
        }
        std::cout << "best_win = " << best_win << "\n";
        std::cout << "o1_cnt = " << o1_cnt << "\n";
        std::cout << "o2_cnt = " << o2_cnt << "\n";
        std::cout << "o3_cnt = " << o3_cnt << "\n";
        std::cout << "o4_cnt = " << o4_cnt << "\n";
        std::cout << "o5_cnt = " << o5_cnt << "\n";
        std::cout << std::setprecision(7);
        std::cout << "o1_time / o1_cnt = " << o1_time / o1_cnt << " ms\n";
        std::cout << "o2_time / o2_cnt = " << o2_time / o2_cnt << " ms\n";
        std::cout << "o3_time / o3_cnt = " << o3_time / o3_cnt << " ms\n";
        std::cout << "o4_time / o4_cnt = " << o4_time / o4_cnt << " ms\n";
        std::cout << "o5_time / o5_cnt = " << o5_time / o5_cnt << " ms\n";
        std::cout << "total_time = " << o1_time + o2_time + o3_time + o4_time + o5_time << " ms\n";
    }

    // операция o1
    // ищет человека с наибольшим выигрышем от перемещения в другую коалицию
    // возвращает увеличение выигрыша
    double o1() {
        Timer timer(o1_cnt, o1_time);
        double max_win = INT32_MIN;
        int max_person, max_coalition;
        for (int person = 0; person < n; ++person) {
            int from_coalition = person_coalition[person];
            int to_coalition = pb_vector[person].max()[0];
            double new_win =
                    pb_vector[person][to_coalition] - pb_vector[person][from_coalition];
            if (new_win > max_win) {
                max_win = new_win;
                max_person = person;
                max_coalition = to_coalition;
            }
        }
        if (max_win <= eps) {
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
        Timer timer(o2_cnt, o2_time);
        o2_updater.reset();
        for (int first_person = 0; first_person < n; ++first_person) {
            int from_first = person_coalition[first_person];
            for (int second_person = 0; second_person < first_person; ++second_person) {
                int from_second = person_coalition[second_person];
                int to_first, to_second;

                // case 1 + case 5
                // два игрока идут в одну коалицию
                // SamePointTransform
                to_first = to_second = same_point_transform[first_person][second_person].max();
                if (to_first == from_first || to_second == from_second) {
                    continue;
                }
                o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
            }
        }
        for (int first_person = 0; first_person < n; ++first_person) {
            int from_first = person_coalition[first_person];
            for (int second_person = 0; second_person < n; ++second_person) {
                if (first_person == second_person) {
                    continue;
                }
                int from_second = person_coalition[second_person];
                const int* potential_to_first = pb_vector[first_person].max();
                const int* potential_to_second = pb_vector[second_person].max();
                int to_first, to_second;
                if (from_first == from_second) {
                    // case 3
                    // два игрока из одной коалиции идут в разные коалиции
                    if (potential_to_first[0] != potential_to_second[0]) {
                        to_first = potential_to_first[0];
                        to_second = potential_to_first[0];
                        o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
                    } else {
                        to_first = potential_to_first[0];
                        to_second = potential_to_first[1];
                        o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
                        to_first = potential_to_first[1];
                        to_second = potential_to_first[0];
                        o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
                    }
                } else {
                    // case 2
                    // два игрока меняются местами
                    to_first = from_second;
                    to_second = from_first;
                    o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);

                    // case 4
                    // второй идёт на место первого, первый идёт в другое место
                    to_second = from_first;
                    if (potential_to_first[0] != from_second) {
                        to_first = potential_to_first[0];
                    } else {
                        to_first = potential_to_first[1];
                    }
                    o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);

                    // case 6
                    // первый идёт на место второго, второй идёт в другое место
                    to_first = from_second;
                    if (potential_to_second[0] != from_first) {
                        to_second = potential_to_second[0];
                    } else {
                        to_second = potential_to_second[1];
                    }
                    o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);

                    // case 7
                    // первый и второй идут из разных в разные
                    for (int one_ind = 0; one_ind < 3; ++one_ind) {
                        to_first = potential_to_first[one_ind];
                        for (int two_ind = 0; two_ind < 3; ++two_ind) {
                            to_second = potential_to_second[two_ind];
                            if (to_first != from_second && to_second != from_first && to_first != to_second) {
                                o2_updater.update(first_person, from_first, to_first, second_person, from_second,
                                                  to_second);
                            }
                        }
                    }
                }
            }
        }
        if (o2_updater.max_win <= eps) {
            return o2_updater.max_win;
        }
#ifdef OPERATION_OUTPUT
        std::cout << "o2" << std::endl;
#endif
        move(o2_updater.max_first_person, o2_updater.max_to_first, true);
        move(o2_updater.max_second_person, o2_updater.max_to_second, true);
        return o2_updater.max_win;
    }

    double o3() {
        Timer timer(o3_cnt, o3_time);
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
            if (tabu_set.count(person) != 0) {
                continue;
            }
            int from_coalition = person_coalition[person];
            int to_coalition = pb_vector[person].max()[0];
            double new_win =
                    pb_vector[person][to_coalition] - pb_vector[person][from_coalition];
            if (new_win > max_win) {
                max_win = new_win;
                max_person = person;
                max_coalition = to_coalition;
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
        Timer timer(o4_cnt, o4_time);
        o2_updater.reset();
        do {
            // случайные коалиции, в которые пойдут игроки
            int to_first = rand() % k;
            int to_second = rand() % k;
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
                    o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
                }
            }
        } while (o2_updater.max_win == INT32_MIN);
#ifdef OPERATION_OUTPUT
        std::cout << "o4" << std::endl;
#endif
        move(o2_updater.max_first_person, o2_updater.max_to_first);
        move(o2_updater.max_second_person, o2_updater.max_to_second);
        return o2_updater.max_win;
    }

    double o5() {
        Timer timer(o5_cnt, o5_time);
        int person = rand() % n;
        int to_coalition = rand() % k;
        int from_coalition = person_coalition[person];
        if (from_coalition == to_coalition) {
            return 0;
        }
        double new_win =
                pb_vector[person][to_coalition] -
                pb_vector[person][from_coalition];
#ifdef OPERATION_OUTPUT
        std::cout << "o5" << std::endl;
#endif
        move(person, to_coalition);
        return new_win;
    }

    void move(int moved_person, int to_coalition, bool update_same_point_transform = false) {
#ifdef MOVE_OUTPUT
        std::cout << "move:\nmoved_person = " << moved_person << "\n";
        std::cout << "from_coalition = " << person_coalition[moved_person] << "\n";
        std::cout << "to_coalition = " << to_coalition << "\n" << std::endl;
#endif
#ifdef DEBUG
        assert(person_coalition[moved_person] != to_coalition);
#endif
        pb_vector.update(moved_person, to_coalition);
        if (update_same_point_transform) {
            same_point_transform.update(moved_person, to_coalition);
        }
        person_coalition[moved_person] = to_coalition;
    }

    double get_real_win() {
        double real_win = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                if (person_coalition[i] == person_coalition[j]) {
                    real_win += w[i][j];
                }
            }
        }
        return real_win;
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
    CoalitionApprox s(w, n, k, p, omega, xi, gamma);
    int n_iter = 100;
    s.run(n_iter);
    s.print_coalitions();
//    std::cin.get();

    return 0;
}
