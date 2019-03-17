# include "genetic2.hpp"
# include "json.hpp"

# include <iostream>
# include <vector>
# include <string>
# include <fstream>

using namespace std;

void toJson(GA2 &ga2) {
    fstream file;
    file.open("output.txt", fstream::out);
    for (int i = 0; i < 5; ++i) {
        file << "Solution " << i + 1 << "(fitness " << ga2.fitnesses[i] << "):\n";
        vector<nlohmann::json> output;
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
    vector<int> sbj_grades, tc_workloads, sbj_workloads;
    vector<vector<int>> out_periods, prefs;

    cin >> sbj_num >> tc_num;

    sbj_grades.resize(sbj_num), sbj_workloads.resize(sbj_num), tc_workloads.resize(tc_num);
    out_periods.resize(tc_num, vector<int>(30));
    prefs.resize(tc_num, vector<int>(sbj_num));

    for (int i = 0; i < sbj_num; ++i) cin >> sbj_grades[i];

    for (int i = 0; i < tc_num; ++i)
        for (int j = 0; j < 30; ++j) cin >> out_periods[i][j];

    for (int i = 0; i < tc_num; ++i)
        for (int j = 0; j < sbj_num; ++j) cin >> prefs[i][j];

    for (int i = 0; i < tc_num; ++i) cin >> tc_workloads[i];
    for (int i = 0; i < sbj_num; ++i) cin >> sbj_workloads[i];

    GA2 ga2(sbj_grades, out_periods, prefs, tc_workloads, sbj_workloads);
    ga2.start();

    toJson(ga2);

    return 0;
}
