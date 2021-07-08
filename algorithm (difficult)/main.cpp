#include "potential_benefits.cpp"
#include "same_point_transformations.cpp"
#include "o2_update.cpp"

#include <chrono>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <vector>

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
    double win = 0;
    double best_win = 0;
    int not_improved_cnt = 0;
    int time = 0;
    int o1_cnt = 0, o2_cnt = 0, o3_cnt = 0, o4_cnt = 0, o5_cnt = 0;
    double time1 = 0, time2 = 0, time3 = 0, time4 = 0, time5 = 0;
    std::vector<std::vector<double>> w;
//    std::vector<std::vector<double>> coalition_interaction; // arr[i][j] сумма рёбер из i игрока в j коалицию
    PB_Vector coalition_interaction;
    SamePointTransform same_point_transform;
    o2Updater o2_updater;
    std::vector<int> person_coalition; // arr[i] - коалиция i игрока
    std::vector<int> person_coalition_answer;
    std::unordered_set<int> tabu_set;
    std::deque<std::pair<int, int>> tabu_list; // (игрок, время окончания бана), снимаем бан, если time > время окончания бана

    CoalitionApprox(std::vector<std::vector<double>> input_w, int n, int k, double p, int omega, int xi, int gamma) :
            n(n), k(k), p(p), omega(omega), xi(xi), gamma(gamma),
            w(std::move(input_w)),
            coalition_interaction(w), same_point_transform(w), o2_updater(coalition_interaction, w),
            person_coalition(n),
            person_coalition_answer(n) {
        // все в 0 коалиции
        // построение coalition_interaction
        std::vector<ThreeMaxTree> coalition_interaction_init(n);
        for (int first_person = 0; first_person < n; ++first_person) {
            std::vector<double> three_max_tree_init(k);
            for (int second_person = 0; second_person < n; ++second_person) {
                three_max_tree_init[0] += w[first_person][second_person];
                win += w[first_person][second_person];
            }
            coalition_interaction_init[first_person].build(std::move(three_max_tree_init), 0);
        }
        coalition_interaction.build(std::move(coalition_interaction_init));
        win /= 2;
        best_win = win;

        // построение same_point_transform
        std::vector<std::vector<MaxTree>> same_point_transform_init(n);
        for (int first_person = 0; first_person < n; ++first_person) {
            same_point_transform_init[first_person] = std::vector<MaxTree>(first_person);
            for (int second_person = 0; second_person < first_person; ++second_person) {
                std::vector<double> max_tree_init(k);
                for (int coalition = 0; coalition < k; ++coalition) {
                    max_tree_init[coalition] = coalition_interaction[first_person][coalition] +
                                               coalition_interaction[second_person][coalition];
                }
                same_point_transform_init[first_person][second_person].build(std::move(max_tree_init), 0, 0);
            }
        }
        same_point_transform.build(std::move(same_point_transform_init));
    }

    static double random() {
        return static_cast<double>(rand() % 1000) / 1000;
    }

    void run(int n_iter) {
        for (int i = 0; i < n_iter; ++i) {
            std::cout << i << " ИТЕРАЦИЯ" << std::endl;
            double inc_win;
#ifdef op1
            // пока улучшается делаем o1
            while (true) {
                auto begin = std::chrono::steady_clock::now();
                inc_win = o1();
                auto end = std::chrono::steady_clock::now();
                time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e6;
                if (inc_win <= 0) {
                    break;
                }
                win += inc_win;
            }
#endif
#ifdef op2
            // пока улучшается делаем o2
            while (true) {
                auto begin = std::chrono::steady_clock::now();
                inc_win = o2();
                auto end = std::chrono::steady_clock::now();
                time2 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e6;
                if (inc_win <= 0) {
                    break;
                }
                win += inc_win;
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
            if (i == n_iter - 1) {
                return;
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
                    auto begin = std::chrono::steady_clock::now();
                    win += o3();
                    auto end = std::chrono::steady_clock::now();
                    time3 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e6;
#endif
                } else {
#ifdef op4
                    auto begin = std::chrono::steady_clock::now();
                    win += o4();
                    auto end = std::chrono::steady_clock::now();
                    time4 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e6;
#endif
                }
                ++move_cnt;
            }
#endif
#ifdef op5
            if (not_improved_cnt >= xi) {
                for (int j = 0; j < gamma; ++j) {
                    auto begin = std::chrono::steady_clock::now();
                    win += o5();
                    auto end = std::chrono::steady_clock::now();
                    time5 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e6;
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
            file << person << " " << person_coalition_answer[person] << "\n";
        }
        std::cout << "best_win = " << best_win << "\n";
        std::cout << "o1_cnt = " << o1_cnt << "\n";
        std::cout << "o2_cnt = " << o2_cnt << "\n";
        std::cout << "o3_cnt = " << o3_cnt << "\n";
        std::cout << "o4_cnt = " << o4_cnt << "\n";
        std::cout << "o5_cnt = " << o5_cnt << "\n";
        std::cout << std::setprecision(7);
        std::cout << "time1 / o1_cnt = " << time1 / o1_cnt << " ms\n";
        std::cout << "time2 / o2_cnt = " << time2 / o2_cnt << " ms\n";
        std::cout << "time3 / o3_cnt = " << time3 / o3_cnt << " ms\n";
        std::cout << "time4 / o4_cnt = " << time4 / o4_cnt << " ms\n";
        std::cout << "time5 / o5_cnt = " << time5 / o5_cnt << " ms\n";
        std::cout << "total_time = " << time1 + time2 + time3 + time4 + time5 << " ms\n";
    }

    // операция o1
    // ищет человека с наибольшим выигрышем от перемещения в другую коалицию
    // возвращает увеличение выигрыша
    double o1() {
        ++o1_cnt;
        double max_win = INT32_MIN;
        int max_person, max_coalition;
        for (int person = 0; person < n; ++person) {
            int from_coalition = person_coalition[person];
            int to_coalition = coalition_interaction[person].max()[0];
            double new_win =
                    coalition_interaction[person][to_coalition] - coalition_interaction[person][from_coalition];
            if (new_win > max_win) {
                max_win = new_win;
                max_person = person;
                max_coalition = to_coalition;
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
        ++o2_cnt;
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
                int to_first, to_second;
                const int* potential_to_first_ptr = coalition_interaction[first_person].max();
                const int* potential_to_second_ptr = coalition_interaction[second_person].max();
                int potential_to_first[3];
                int potential_to_second[3];
                std::copy(potential_to_first_ptr, potential_to_first_ptr + 3, potential_to_first);
                std::copy(potential_to_second_ptr, potential_to_second_ptr + 3, potential_to_second);
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
                    for (int one_ind : potential_to_first) {
                        for (int two_ind : potential_to_second) {
                            to_first = one_ind;
                            to_second = two_ind;
                            if (to_first != from_second && to_second != from_first && to_first != to_second) {
                                o2_updater.update(first_person, from_first, to_first, second_person, from_second, to_second);
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (o2_updater.max_win <= 0) {
            return o2_updater.max_win;
        }
#ifdef OPERATION_OUTPUT
        std::cout << "o2" << std::endl;
#endif
        move(o2_updater.max_first_person, o2_updater.max_to_first);
        move(o2_updater.max_second_person, o2_updater.max_to_second);
        return o2_updater.max_win;
    }

    double o3() {
        ++o3_cnt;
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
            int to_coalition = coalition_interaction[person].max()[0];
            double new_win =
                    coalition_interaction[person][to_coalition] - coalition_interaction[person][from_coalition];
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
        ++o4_cnt;
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
        ++o5_cnt;
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

    void move(int moved_person, int to_coalition) {
#ifdef MOVE_OUTPUT
        std::cout << "move:\nmoved_person = " << moved_person << "\n";
        std::cout << "from_coalition = " << person_coalition[moved_person] << "\n";
        std::cout << "to_coalition = " << to_coalition << "\n" << std::endl;
#endif
        coalition_interaction.update(moved_person, to_coalition);
        same_point_transform.update(moved_person, to_coalition);
        person_coalition[moved_person] = to_coalition;
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
    std::cin.get();

    return 0;
}
