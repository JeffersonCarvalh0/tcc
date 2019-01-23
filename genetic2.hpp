# ifndef GENETIC2_HPP
# define GENETIC2_HPP

# include "utils.hpp"

# include <map>
# include <memory>
# include <list>
# include <random>
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
    std::vector<Tuple> tuples;
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
        population = std::vector<std::vector<std::list<Tuple>>>(
            pop_size, std::vector<std::vector<Tuple>>(periods_size, std::list<Tuple>);
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

        // Calculates the fitness of each chromossome
        for (int i = 0; i < pop_size; ++i) fitnesses[i] = fitness(i);
    }

    /* Calculates the finess of a chromossome(solution) */
    int fitness(int ch) {
        int cost = 0;
        
    }
};

# endif /* end of include guard: GENETIC2_HPP */
