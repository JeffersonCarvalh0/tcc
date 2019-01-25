# ifndef GENETIC2_HPP
# define GENETIC2_HPP

# include "utils.hpp"

# include <algorithm>
# include <map>
# include <memory>
# include <list>
# include <random>
# include <set>
# include <vector>

using int_matrix = std::vector<std::vector<int>>;
using int_vector = std::vector<int>;

/*
    A generic genetic algorithm to solve the STP. Its initial population can be
    random or it can be the outputs of the first GA executed for each teacher.
*/
class GA2 {
public:
    int pop_size, periods_size, periods_per_day;
    std::vector<std::vector<std::list<int>>> population;
    std::vector<Tuple> &tuples;
    int_matrix &out_periods, &prefs;
    std::vector<std::list<int>> teacher_subject;
    int_vector fitnesses, &tc_max_workloads, &sbj_workloads, &first_labels, tc_cur_workloads;
    std::vector<bool> allocated_subjects;

    std::random_device rd;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> random_period;
    std::uniform_int_distribution<int> random_teacher;

private:
    void setTuples(int teacher, int subject) {
        for (int i = first_labels[subject]; i < first_labels[subject] + sbj_workloads[subject]; ++i)
            tuples[i].teacher = teacher;
    }

public:
    // Constructor with random initial population
    GA2(std::vector<Tuple> &tuples, int_matrix &out_periods, int_matrix &prefs, int_vector &tc_max_workloads, int_vector &sbj_workloads, int_vector first_labels, int pop_size, int periods_size = 30, int periods_per_day = 6):
    tuples(tuples), out_periods(out_periods), prefs(prefs), tc_max_workloads(tc_max_workloads), sbj_workloads(sbj_workloads), first_labels(first_labels), pop_size(pop_size), periods_size(periods_size), periods_per_day(periods_per_day) {

        // Initilaizing the attributes
        generator = std::default_random_engine(rd());
        random_period = std::uniform_int_distribution<int>(0, periods_size - 1);
        population = std::vector<std::vector<std::list<int>>>(
            pop_size, std::vector<std::vector<int>>(periods_size, std::list<int>());
        );
        fitnesses.resize(pop_size);
        tc_cur_workloads.resize(tc_max_workloads.size(), 0);
        allocated_disciplines(sbj_workloads.size(), false);
        teacher_subject.resize(tc_max_workloads.size());

        // Randomly setting disciplines to the teachers
        random_teacher = std::uniform_int_distribution<int>(0, tc_max_workloads.size() - 1);
        for (int i = 0; i < (int)sbj_workloads.size(); ++i) {
            int chosen_teacher = random_teacher(generator);
            tc_cur_workloads[chosen_teacher] += sbj_workloads[i];
            allocated_subjects[i] = true;
            teacher_subject[chosen_teacher].push_back(i);
            setTuples(chosen_teacher, i);
        }

        // Creating the first generation
        for (int i = 0; i < pop_size; ++i) {
            for (int j = 0; i < (int)tuples.size(); ++j) {
                int chosen_period;
                chosen_period = random_period(generator);
                population[i][chosen_period].push_back(j);
            }
        }

        // Calculates the fitness of each chromossome
        for (int i = 0; i < pop_size; ++i) fitnesses[i] = fitness(i);
    }

    /* Calculates the finess of a chromossome(solution) */
    int fitness(int ch) {
        int cost = 0;

        // Checking for lectures of the same subject in the same day
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            std::map<int, int> m;
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                for (auto &label : population[ch][j]) {
                    if (m.find(tuples[label].subject) == m.end())
                        m.insert({ tuples[label].subject, 1 });
                    else
                        ++m[tuples[label].subject];
                }
            }
            for (auto &e : m) if (e.second > 1) cost += (10 * (e.second - 1));
        }

        // Checking for windows in the timetable
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                std::set<int> today, yesterday, tomorrow;

                if (j > i * periods_per_day && j < (i + 1) * periods_per_day) {
                    for (auto &label : population[ch][j]) today.insert(tuples[label].teacher);
                    for (auto &label : population[ch][j - 1]) yesteday.insert(tuples[label].teacher);
                    for (auto &label : population[ch][j + 1]) tomorrow.insert(tuples[label].teacher);

                    for (auto &teacher : yesteday) {
                        if (tomorrow.find(teacher) != tomorrow.end() && today.find(teacher) == today.end())
                            cost += 10;
                    }
                }
            }
        }

        // Checking for teachers lecturing in out periods
        for (int i = 0; i < periods_size; ++i) {
            for (auto &label : population[ch][i]) {
                if (out_priods[tuples[label].teacher][i]) cost += 10;
            }
        }

        // Checking for teacher's subjects preferences
        for (int i = 0; i < (int)teacher_subject.size(); ++i) {
            for (auto &subject : teacher_subject[i]) {
                switch(prefs[i][subject]) {
                    case 4 : cost += 2; break;
                    case 3 : cost += 4; break;
                    case 2 : cost += 6; break;
                    case 1 : cost += 10; break;
                }
            }
        }

        return 500 - (cost > 500 ? 500 : cost);
    }

    /*
        Probabilistically selects the parents based on their fitnesses
        Roulette Wheel method
    */
    void select(int &parent1, int &parent2) {
        int total = 0;
        for (int i = 0; i < pop_size; ++i) total += fitnesses[i];

        std::uniform_int_distribution<int> v(1, total);
        int chosen = v(generator);

        for (int i = 0; chosen > 0; ++i) {
            chosen -= fitnesses[i];
            if (chosen <= 0) { parent1 = i; break; }
        }

        chosen = v(generator);
        for (int i = 0; chosen > 0; ++i) {
            chosen -= fitnesses[i];
            if (chosen <= 0) { parent2 = i; break; }
        }
    }

    /* Checks for hard constraints violated and fix them */
    void fix(std::vector<list<int>> &child) {
        std::vector<bool> found(tuples.size(), false);

        // Checks for missing tuples and place them randomly
        for (auto &period : child)
            for (auto &tuple : period) found[tuple] = true;
        for (int i = 0; i < int(found.size()); ++i) {
            if (!found[i]) {
                int chosen = random_period(generator);
                child[chosen].push_back(tuples[i].label);
            }
        }

        // Checks for repeated tuples and remove them
        found.assign(found.size(), false);
        for (auto &period : child) {
            for (auto it = period.begin(); it != period.end(); ++it) {
                if (!found[*it]) found[*it] = true;
                else period.erase(it);
            }
        }

        // Checks for same teacher and same grade in the same period and moves elsewhere
        for (auto &period : child) {
            std::set<int> teachers, grades;
            for (auto &tuple : period) {
                int cur_teacher = tuples[tuple].teacher;
                int cur_grade = tuples[tuple].grade;
                if (teachers.find(cur_teacher) != teachers.end() || grades.find(cur_grade) != grades.end()) {
                    do new_period = random_period(generator);
                    while (std::none_of(
                        child[new_period].begin(),
                        child[new_period].end(),
                        [this, cur_teacher](int t) {
                            return tuples[t].teacher == cur_teacher || tuples[t].grade == cur_grade;
                        }
                    )); // Might become an infinite loop it the teacher's workload is too high and he has classes in every period
                } else teachers.insert(cur_teacher), grades.insert(cur_grade);
            }
        }
    }

    /* Creates two children from a pair of parents using uniform technique */
    void crossover(int parent1, int parent2, std::vector<list<int>> &child1, std::vector<list<int>> &child2) {
        std::uniform_int_distribution<int> coin(1, 100);

        for (int i = 0; i < periods_size; ++i) {
            if (coin(generator) <= 50) {
                child1[i] = population[parent1][i];
                child2[i] = population[parent2][i];
            } else {
                child1[i] = population[parent2][i];
                child2[i] = population[parent1][i];
            }
        }

        fix(child1); fix(child2);
    }

    /*
        An mutation operator that randomly selects two tuples from two periods
        and switch them
    */
    void swap_tuples(std::vector<list<int>> &child) {
        std::uniform_int_distribution<int> m(1, 100);
        if (m(generator) <= 5) {
            int p1, p2;
            do p1 = random_period(generator); while (child[t1] == -1);
            do p2 = random_period(generator); while (child[t2] == -1);

            std::swap(child[t1], child[t2]);
        }
    }

    /*
        An mutation operator that swap the disciplines allocated for teachers
    */
    void swap_teachers_disciplines() {
        std::uniform_int_distribution<int> m(1, 100);
        if (m(generator == 5)) {
            
        }
    }
};

# endif /* end of include guard: GENETIC2_HPP */
