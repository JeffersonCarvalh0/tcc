# ifndef UTILS_HPP
# define UTILS_HPP

# include <map>
# include <vector>
# include <set>

/* Tuples will be assigned to periods */
struct Tuple {
    int label, teacher, subject, grade;

    Tuple(int label = -1, int teacher = -1, int subject = -1, int grade = -1):
        label(label), teacher(teacher), subject(subject), grade(grade) {}
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

# endif /* end of include guard: UTILS_HPP */
