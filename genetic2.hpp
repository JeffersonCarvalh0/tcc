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
private:
    struct Chromossome {
        std::vector<std::list<Tuple>> periods;
        std::vector<std::vector<bool>> teacher_subject;
        std::vector<int> tc_cur_workloads;

        Chromossome(int periods_size, int teachers_size, int subjects_size) {
            periods.resize(periods_size);
            teacher_subject.resize(teachers_size, std::vector<bool>(subjects_size, false));
            tc_cur_workloads.resize(subjects_size);
        }
    };

private:
    void setTuples(Chromossome &ch, int teacher, int subject) {
        for (auto &period : ch.periods) {
            for (auto &tuple : period) {
                if (tuple.subject == subject) tuple.teacher = teacher;
            }
        }
    }

    void swap_teachers(Chromossome &ch, int sbj1, int sbj2) {
        int teacher1, teacher2;
        for (teacher1 = 0; teacher1 < (int)tc_max_workloads.size(); ++teacher1)
            if (ch.teacher_subject[teacher1][sbj1]) break;

        for (teacher2 = 0; teacher2 < (int)tc_max_workloads.size(); ++teacher2)
            if (ch.teacher_subject[teacher2][sbj2]) break;

        ch.teacher_subject[teacher1][sbj1] = false;
        ch.teacher_subject[teacher1][sbj2] = true;
        ch.teacher_subject[teacher2][sbj2] = false;
        ch.teacher_subject[teacher2][sbj1] = true;

        ch.tc_cur_workloads[teacher1] -= sbj_workloads[sbj1];
        ch.tc_cur_workloads[teacher1] += sbj_workloads[sbj2];
        ch.tc_cur_workloads[teacher2] -= sbj_workloads[sbj2];
        ch.tc_cur_workloads[teacher2] += sbj_workloads[sbj1];

        setTuples(ch, teacher1, sbj1);
        setTuples(ch, teacher2, sbj2);
    }

public:
    int pop_size, periods_size, periods_per_day;
    std::vector<Chromossome> population;
    int_matrix &out_periods, &prefs;
    int_vector fitnesses, &tc_max_workloads, &sbj_workloads, &sbj_grades;

    std::random_device rd;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> random_period;
    std::uniform_int_distribution<int> random_teacher;
    std::uniform_int_distribution<int> random_subject;

public:
    // Constructor with random initial population
    GA2(int_vector &sbj_grades, int_matrix &out_periods, int_matrix &prefs, int_vector &tc_max_workloads, int_vector &sbj_workloads, int_vector first_labels, int pop_size = 50, int periods_size = 30, int periods_per_day = 6):
    sbj_grades(sbj_grades), out_periods(out_periods), prefs(prefs), tc_max_workloads(tc_max_workloads), sbj_workloads(sbj_workloads), pop_size(pop_size), periods_size(periods_size), periods_per_day(periods_per_day) {

        // Initilaizing the attributes
        generator = std::default_random_engine(rd());
        random_period = std::uniform_int_distribution<int>(0, periods_size - 1);
        population = std::vector<Chromossome>(
            pop_size,
            Chromossome(periods_size, tc_max_workloads.size(), sbj_workloads.size())
        );
        fitnesses.resize(pop_size);

        // Randomly setting disciplines to the teachers for each solution
        random_teacher = std::uniform_int_distribution<int>(0, tc_max_workloads.size() - 1);
        for (int ch = 0; ch < pop_size; ++ch) {
            for (int j = 0; j < (int)sbj_workloads.size(); ++j) {
                int chosen_teacher = random_teacher(generator);
                population[ch].tc_cur_workloads[chosen_teacher] += sbj_workloads[j];
                population[ch].teacher_subject[chosen_teacher][j] = true;
            }
        }

        // Creating the first generation
        random_subject = std::uniform_int_distribution<int>(0, sbj_workloads.size() - 1);
        for (int ch = 0; ch < pop_size; ++ch) {
            for (int tc_idx = 0; tc_idx < (int)tc_max_workloads.size(); ++tc_idx) {
                int chosen_period = random_period(generator);
                int chosen_subject = random_subject(generator);
                population[ch].periods[chosen_period].push_back(Tuple(-1, tc_idx, chosen_subject, sbj_grades[chosen_subject]));
                population[ch].teacher_subject[tc_idx][chosen_subject] = true;
            }
            fix(population[ch]);
        }

        // Calculates the fitness of each chromossome
        for (int i = 0; i < pop_size; ++i) fitnesses[i] = fitness(population[i]);
    }

    /* Calculates the finess of a chromossome(solution) */
    int fitness(const Chromossome &ch) {
        int cost = 0;

        // Checking for lectures of the same subject in the same day
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            std::map<int, int> m;
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                for (auto &tuple : ch.periods[j]) {
                    if (m.find(tuple.subject) == m.end())
                        m.insert({ tuple.subject, 1 });
                    else
                        ++m[tuple.subject];
                }
            }
            for (auto &e : m) if (e.second > 1) cost += (10 * (e.second - 1));
        }

        // Checking for windows in the timetable
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                std::set<int> today, yesterday, tomorrow;

                if (j > i * periods_per_day && j < (i + 1) * periods_per_day) {
                    for (auto &tuple : ch.periods[j]) today.insert(tuple.teacher);
                    for (auto &tuple : ch.periods[j - 1]) yesterday.insert(tuple.teacher);
                    for (auto &tuple : ch.periods[j + 1]) tomorrow.insert(tuple.teacher);

                    for (auto &teacher : yesterday) {
                        if (tomorrow.find(teacher) != tomorrow.end() && today.find(teacher) == today.end())
                            cost += 10;
                    }
                }
            }
        }

        // Checking for teachers lecturing in out periods
        for (int i = 0; i < periods_size; ++i) {
            for (auto &tuple : ch.periods[i]) {
                if (out_periods[tuple.teacher][i]) cost += 10;
            }
        }

        // Checking for teacher's subjects preferences
        for (int i = 0; i < (int)tc_max_workloads.size(); ++i) {
            for (int j = 0; j < (int)sbj_workloads.size(); ++j) {
                if (ch.teacher_subject[i][j]) {
                    switch(prefs[i][j]) {
                        case 4 : cost += 2; break;
                        case 3 : cost += 4; break;
                        case 2 : cost += 6; break;
                        case 1 : cost += 10; break;
                    }
                }
            }
        }

        // Checks for teachers abor maximum workload
        for (int i = 0; i < (int)tc_max_workloads.size(); ++i) {
            if (ch.tc_cur_workloads[i] > tc_max_workloads[i])
                cost += ch.tc_cur_workloads[i] - tc_max_workloads[i];
        }

        return 500 - (cost > 500 ? 500 : cost);
    }

    /*
        Probabilistically selects the parents based on their fitnesses
        Roulette Wheel method
    */
    void select(int &parent) {
        int total = 0;
        for (int i = 0; i < pop_size; ++i) total += fitnesses[i];

        std::uniform_int_distribution<int> v(1, total);
        int chosen = v(generator);

        for (int i = 0; chosen > 0; ++i) {
            chosen -= fitnesses[i];
            if (chosen <= 0) { parent = i; break; }
        }
    }

    /* Checks for hard constraints violated and fix them */
    void fix(Chromossome &child) {
        std::vector<int> count(sbj_workloads.size());

        // Checks for missing tuples and place them randomly
        for (auto &period : child.periods)
            for (auto &tuple : period) ++count[tuple.subject];
        for (int i = 0; i < int(count.size()); ++i) {
            while (count[i] < sbj_workloads[i]) {
                int chosen = random_period(generator);
                int teacher;
                for (teacher = 0; teacher < (int)tc_max_workloads.size(); ++teacher)
                    if (child.teacher_subject[teacher][i]) break;
                child.periods[chosen].push_back(Tuple(-1, teacher, i, sbj_grades[i]));
                ++count[i];
            }
        }

        // Checks for extra tuples and remove them
        for (int i = 0; i < (int)count.size(); ++i) {
            while (count[i] > sbj_workloads[i]) {
                for (auto &period : child.periods) {
                    auto tuple = std::find_if(period.begin(), period.end(),
                        [i](Tuple &tuple) { return tuple.subject == i; }
                    );
                    period.erase(tuple); --count[i];
                }
            }
        }

        // Checks for same teacher and same grade in the same period and moves elsewhere
        for (auto &period : child.periods) {
            std::set<int> teachers, grades;
            for (auto tuple = period.begin(); tuple != period.end(); ++tuple) {
                int cur_teacher = tuple->teacher, cur_grade = tuple->grade;
                if (teachers.find(cur_teacher) != teachers.end() || grades.find(cur_grade) != grades.end()) {
                    int new_period;
                    do new_period = random_period(generator);
                    while (std::none_of(
                        child.periods[new_period].begin(), child.periods[new_period].end(),
                        [cur_teacher, cur_grade](Tuple &t) {
                            return t.teacher == cur_teacher || t.grade == cur_grade;
                        }
                    )); // Might become an infinite loop it the teacher's workload is too high and he has classes in every period
                    child.periods[new_period].push_back(std::move(*tuple));
                    period.erase(tuple++); --tuple;
                } else teachers.insert(cur_teacher), grades.insert(cur_grade);
            }
        }
    }

    /*
        An mutation operator that randomly selects two tuples from two periods
        and switch them
    */
    Chromossome swap_tuples(Chromossome &parent) {
        Chromossome child = parent;
        std::uniform_int_distribution<int> m(1, 100);
        int p1, p2;

        p1 = random_period(generator);
        do p2 = random_period(generator); while (p2 == p1);
        std::swap(child.periods[p1], child.periods[p2]);

        return child;
    }

    /*
        An mutation operator that randomply swap the subjects allocated for
        teachers
    */
    Chromossome swap_teachers_subjects(Chromossome &parent) {
        Chromossome child = parent;
        std::uniform_int_distribution<int> m(1, 100);
        int sbj1, sbj2;

        sbj1 = random_subject(generator);
        do sbj2 = random_subject(generator); while (sbj2 == sbj1);
        swap_teachers(child, sbj1, sbj2);

        return child;
    }

    bool mutation(int parent, Chromossome &child) {
        std::uniform_int_distribution<int> coin(1, 100);

        if (coin(generator) <= 50) child = std::move(swap_tuples(population[parent]));
        else child = std::move(swap_teachers_subjects(population[parent]));

        if (fitness(child) > fitnesses[parent]) return true;
        return false;
    }

    /* Creates a new generation */
    void breed() {
        std::vector<Chromossome> new_population(
            pop_size,
            Chromossome(periods_size, tc_max_workloads.size(), sbj_workloads.size())
        );
        int survivors_qtd = pop_size - int(pop_size * 0.95);

        // Select survivors using elitism
        std::multimap<int, int> best;
        for (int i = 0; i < pop_size; ++i) best.insert({ fitnesses[i], i });

        int i = 0;
        for (auto it = best.rbegin(); it != best.rend() && i < survivors_qtd; ++it, ++i)
            new_population[i] = population[it->second];

        // Probabilistically chooses the parent
        int parent; select(parent);

        // Generate the rest of the new population through mutations
        Chromossome child(periods_size, tc_max_workloads.size(), sbj_workloads.size());
        for (int i = survivors_qtd; i < pop_size; ++i)
            if (mutation(parent, child)) new_population[i] = std::move(child);

        population = std::move(new_population);

        // Calculates the fitnesses of the new generation
        for (int i = 0; i < pop_size; ++i) fitnesses[i] = fitness(population[i]);
    }

    void start() {
        while (!std::all_of(fitnesses.begin(), fitnesses.end(),
        [](int fitness){ return fitness == 500; })) {
            breed();
        }
    }
};

# endif /* end of include guard: GENETIC2_HPP */
