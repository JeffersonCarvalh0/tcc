# include "knapsack.hpp"
# include "genetic1.hpp"
# include "genetic2.hpp"
# include "utils.hpp"
# include "json.hpp"

# include <iostream>
# include <fstream>
# include <algorithm>
# include <map>
# include <set>
# include <vector>

using namespace std;

void printChromossomes(GA1 &ga1) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < ga1.periods_size; ++j)
            cout << ga1.population[i][j] << ' ';
        cout << "(fitness " << ga1.fitnesses[i] << ")\n";
    }
}

void toJson(GA2 &ga2) {
    fstream file;
    file.open("output.txt", fstream::out);
    for (int i = 0; i < 5; ++i) {
        file << "Solution " << i + 1 << "(fitness " << ga2.fitnesses[i] << "):\n";
        std::vector<nlohmann::json> output;
        for (int j = 0; j < ga2.population[i].periods.size(); ++j) {
            for (auto &tuple : ga2.population[i].periods[j]) {
                nlohmann::json obj;
                obj["period"] = j;
                obj["teacher"] = tuple.teacher;
                obj["subject"] = tuple.subject;
                obj["grade"] = tuple.grade;
                output.push_back(obj);
            }
        }
        file << nlohmann::json(output) << "\n\n";
    }

    file.close();
}

int main() {
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
        cout << "Executing GA1 for each teacher (" << i + 1 << '/' << tc_num << ")\n";
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
                for (auto &label : solution) label != -1
                    ? converted.push_back(tuples[label])
                    : converted.push_back(Tuple());

                return converted;
            }
        );

        printChromossomes(ga1);
        results.insert(results.end(), result.begin(), result.end());
        cout << '\n';
    }

    cout << "Done.\n";
    vector<Chromossome> ga2_input = GA1toGA2(results, prefs, sbj_num, tc_num);

    cout << "Executing GA2...\n";
    GA2 ga2(sbj_grades, out_periods, prefs, tc_max_workloads, sbj_workloads, ga2_input);
    ga2.start();

    return 0;
}
