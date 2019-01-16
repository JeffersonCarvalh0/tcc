# include "knapsack.hpp"
# include "genetic1.hpp"

# include <iostream>
# include <iomanip>
# include <map>

using namespace std;

ostream& operator << (ostream &stream, Tuple &tuple) {
    stream << setw(2) << tuple.label << setw(5) << tuple.teacher << setw(8) << tuple.subject << setw(8) << tuple.grade;
    return stream;
}

void printChromossomesTerminal(GA1 &ga1) {
    cout << "Tuples:\n";
    cout << "label" << ' ' << "teacher" << ' ' << "subject" << ' ' << "grade" << '\n';
    for (auto &tuple : ga1.tuples) cout << tuple << '\n';

    cout << '\n';

    for (int i = 1; i <= 5; ++i) {
        cout << "solution " << i << "(fitness " << ga1.fitnesses[i - 1] << "):\n";
        for (int j = 1; j <= 6; ++j) {
            cout << "period " << j << ": ";
            for (int k = j - 1; k < 30; k += 6)
                cout << setw(3) << ga1.population[i - 1][k].label << ' ';
            cout << '\n';
        }
        cout << "\n";
    }
}

void printChromossomes(GA1 &ga1) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < ga1.periods_size; ++j)
            cout << ga1.population[i][j].label << ' ';
        cout << "(fitness " << ga1.fitnesses[i] << ")\n";
    }
}

int main() {
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
    int wl, n, out_periods_num;
    vector<int> workloads, prefs, grades;
    vector<bool> out_periods;
    map<int, int> subjects;
    set<int> chosen_subs;

    cin >> wl >> n;
    workloads.resize(n); prefs.resize(n); grades.resize(n);
    out_periods.resize(30);

    // Reading the input
    for (int i = 0; i < n; ++i) cin >> workloads[i];
    for (int i = 0; i < n; ++i) cin >> prefs[i];
    for (int i = 0; i < n; ++i) cin >> grades[i];

    cin >> out_periods_num;
    int cur;
    for (int i = 0; i < out_periods_num; ++i) cin >> cur, out_periods[cur] = true;

    // Creating a map between subjects and its grades
    for (int i = 0; i < n; ++i) subjects.insert({ i, grades[i] });

    // Choosing the best subjects
    chosen_subs = knapsack(n, wl, workloads, prefs);

    // Executing the first GA
    // Getting all possible tuples
    vector<Tuple> tuples = createTuples(0, chosen_subs, subjects, workloads);

    // Initializing the algorithm
    GA1 ga1(tuples, workloads, out_periods);

    // Refining the initial population
    ga1.start();

    // Outputs the results
    printChromossomes(ga1);

    return 0;
}
