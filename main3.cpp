# include "knapsack.hpp"
# include "genetic1.hpp"
# include "utils.hpp"

# include <iostream>
# include <algorithm>
# include <map>
# include <set>
# include <vector>

using namespace std;

int main() {
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

    int sbj_num, tc_num;
    vector<int> sbj_grades, tc_max_workloads, sbj_workloads;
    vector<vector<int>> prefs, out_periods;
    vector<vector<Tuple>> results;
    map<int, int> subjects;

    cin >> sbj_num >> tc_num;
    sbj_grades.resize(sbj_num); tc_max_workloads.resize(tc_num); sbj_workloads.resize(sbj_num);
    out_periods.resize(tc_num, vector<int>(30)); prefs.resize(tc_num, vector<int>(sbj_num));

    // Inputs
    for (int i = 0; i < sbj_num; ++i) cin >> sbj_grades[i];

    for (int i = 0; i < tc_num; ++i)
        for (int j = 0; j < 30; ++j) cin >> out_periods[i][j];

    for (int tc_idx = 0; tc_idx < tc_num; ++tc_idx)
        for (int sbj_idx = 0; sbj_idx < sbj_num; ++sbj_idx) cin >> prefs[tc_idx][sbj_idx];

    for (int i = 0; i < tc_num; ++i) cin >> tc_max_workloads[i];
    for (int i = 0; i < sbj_num; ++i) cin >> sbj_workloads[i];
    for (int i = 0; i < sbj_num; ++i) subjects[i] = sbj_grades[i];

    // Execute the GA1 for every teacher
    for (int i = 0; i < tc_num; ++i) {
        set<int> chosen_subs = knapsack(sbj_num, tc_max_workloads[i], sbj_workloads, prefs[i]);
        vector<Tuple> tuples = createTuples(i, chosen_subs, subjects, sbj_workloads);

        GA1 ga1(tuples, sbj_workloads, out_periods[i]);
        ga1.start();

        vector<vector<Tuple>> result(ga1.pop_size / tc_num);
        transform(
            ga1.population.begin(),
            ga1.population.begin() + ga1.pop_size / tc_num,
            result.begin(),
            [&tuples](vector<int> &solution){
                vector<Tuple> converted;
                for (auto &label : solution) converted.push_back(tuples[label]);
                return converted;
            }
        );

        results.insert(results.end(), result.begin(), result.end());
    }

    vector<Chromossome> ga2_input = GA1toGA2(results, prefs, sbj_num, tc_num);

    return 0;
}
