#include <iostream>
#include <exception>

struct o2Updater {
    const PB_Vector& coalition_interaction;
    const std::vector<std::vector<double>>& w;
    double max_win = INT32_MIN;
    int max_first_person = INT32_MIN, max_second_person = INT32_MIN;
    int max_to_first = INT32_MIN, max_to_second = INT32_MIN;

    o2Updater(const PB_Vector& coalition_interaction, std::vector<std::vector<double>>& w) :
            coalition_interaction(coalition_interaction), w(w) {}

    void reset() {
        max_win = INT32_MIN;
        max_first_person = INT32_MIN, max_second_person = INT32_MIN;
        max_to_first = INT32_MIN, max_to_second = INT32_MIN;
    }

    void update(int first_person, int from_first, int to_first, int second_person, int from_second, int to_second) {
        if (to_second == INT32_MIN || to_first == INT32_MIN) {
            return;
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

    static int multiplier(int from_first, int from_second, int to_first, int to_second) {
        // from_first -> to_first
        // from_second -> to_second

        // case 1
        if (from_first == from_second &&
            to_first == to_second) {
            return 2;
        }
        // case 2
        if (from_first == to_second &&
            from_second == to_first) {
            return -2;
        }
        // case 3
        if (from_first == from_second &&
            to_first != to_second) {
            return 1;
        }
        // case 4
        if (from_first == to_second &&
            to_first != from_second) {
            return -1;
        }
        // case 5
        if (from_first != from_second &&
            to_first == to_second) {
            return 1;
        }
        // case 6
        if (to_first == from_second &&
            from_first != to_second) {
            return -1;
        }
        // case 7
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
        throw std::exception();
    }
};