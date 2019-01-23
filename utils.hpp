# ifndef UTILS_HPP
# define UTILS_HPP

# include <map>
# include <vector>

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

/* This funciton will create the tuples that needs to be assigned for the second GA */
std::vector<Tuple> createTuples(int teachers, std::map<int, int> &subjects, std::vector<int> &workloads) {
    std::vector<Tuple> tuples;
    tuples.reserve(subjects.size());

    int cur_label = 0;
    for (auto &subject : subjects)
        for (int i = 0; i < workloads[subject.first]; ++i)
            tuples.push_back(Tuple(cur_label++, -1, subject.first, subject.second));

    return tuples;
}

/* Returns a vector with the label of the first tuple of each subject */
std::vector<int> firstLabel(std::vector<Tuple> &tuples, std::vector<int> &workloads) {
    std::vector<int> first_labels(workloads.size());
    int cur_subject = 0;

    for (int i = 0; i < (int)tuples.size(); i += workloads[cur_subject++])
        first_labels.push_back(i);

    return first_labels;
}

# endif /* end of include guard: UTILS_HPP */
