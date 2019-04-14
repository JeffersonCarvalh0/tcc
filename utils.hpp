# ifndef UTILS_HPP
# define UTILS_HPP

# include <list>
# include <map>
# include <random>
# include <set>
# include <vector>

/* Tuples will be assigned to periods */
struct Tuple {
    int label, teacher, subject, grade;

    Tuple(int label = -1, int teacher = -1, int subject = -1, int grade = -1):
        label(label), teacher(teacher), subject(subject), grade(grade) {}
};

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

/* This funciton will create the tuples that needs to be assigned for the first GA */
std::vector<Tuple> createTuples(int teacher, std::set<int> &chosen_subjects, std::map<int, int> &subjects, std::vector<int> &workloads) {
    std::vector<Tuple> tuples;

    int total = 0;
    for (auto &e : chosen_subjects) total += workloads[e];
    tuples.reserve(total);

    int cur_label = 0;
    for (auto &e : chosen_subjects) {
        for (int i = 0; i < workloads[e]; ++i)
            tuples.push_back(Tuple(cur_label++, teacher, e, subjects[e]));
    }

    return tuples;
}

/* Converts output of all GA1 runs into an input of GA2 */
std::vector<Chromossome> GA1toGA2(
    std::vector<std::vector<Tuple>> &input_periods, std::vector<std::vector<int>> &prefs,
    int sbj_num, int tc_num, int periods_size = 30, int pop_size = 50
    ) {
    std::vector<Chromossome> population(pop_size, Chromossome(periods_size, tc_num, sbj_num));
    int sols_per_tc = pop_size / tc_num;

    // Placing the tuples of GA1 at GA2
    for (int i = 0; i < tc_num; ++i) {
        for (int j = i * sols_per_tc; j < (i + 1) * sols_per_tc; ++j) {
            for (int z = 0; z < periods_size; ++z) {
                if (input_periods[i][z].label != -1)
                    population[j].periods[z].push_back(input_periods[i][z]);
            }
        }
    }

    // Set the remaining subjects to other teachers
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> random_teacher(0, tc_num - 1);
    for (auto &chromossome : population) {
        for (int sbj_id = 0; sbj_id < sbj_num; ++sbj_id) {
            bool found = false;
            for (int tc_id = 0; tc_id < tc_num; ++tc_id)
                if (chromossome.teacher_subject[tc_id][sbj_id]) {found = true; break;}
            if (!found) chromossome.teacher_subject[random_teacher(generator)][sbj_id] = true;
        }
    }

    // Populate teacher_subject
    for (auto &chromossome : population) {
        for (auto &period : chromossome.periods)
            for (auto &tuple : period)
                chromossome.teacher_subject[tuple.teacher][tuple.subject] = true;
    }

    // Check for subjects with extra teachers	
    for (auto &chromossome : population) {
        for (int sbj_id = 0; sbj_id < sbj_num; ++sbj_id) {
            int chosen_tc_id = -1;
            for (int tc_id = 0; tc_id < tc_num; ++tc_id) {
                if (chromossome.teacher_subject[tc_id][sbj_id]) {
                    if (chosen_tc_id == -1) chosen_tc_id = tc_id;
                    else {
                        prefs[tc_id][sbj_id] > prefs[chosen_tc_id][sbj_id] ?
                            chromossome.teacher_subject[chosen_tc_id][sbj_id] = false, chosen_tc_id = tc_id :
                            chromossome.teacher_subject[tc_id][sbj_id] = false;
                    }
                }
            }
        }
    }

    // Check for subjects with no teacher	
    for (auto &chromossome : population) {
        for (int sbj_id = 0; sbj_id < sbj_num; ++sbj_id) {
            bool has_teacher = false;
            for (int tc_id = 0; tc_id < tc_num; ++tc_id) {
                if (chromossome.teacher_subject[tc_id][sbj_id]) {
                    has_teacher = true;
                    break;
                }
            }
            if (!has_teacher)
                chromossome.teacher_subject[random_teacher(generator)][sbj_id] = true;
        }
    }

    // Set teacher workloads 
    for (auto &chromossome : population) {
        for (int tc_id = 0; tc_id < tc_num; ++tc_id) {
            int classes = 0;
            for (int sbj_id = 0; sbj_id < sbj_num; ++sbj_id)
                if (chromossome.teacher_subject[tc_id][sbj_id]) ++classes;
            chromossome.tc_cur_workloads[tc_id] = classes;	
        }
    }
    return population;
}

# endif /* end of include guard: UTILS_HPP */
