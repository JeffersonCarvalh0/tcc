# include "knapsack.hpp"
# include "genetic1.hpp"

# include <map>

using namespace std;

ostream& operator << (ostream &stream, Tuple &tuple) {
    stream << tuple.label << ' ' << tuple.teacher << ' ' << tuple.subject << ' ' << tuple.grade;
    return stream;
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

    // Reading the inputs
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
    cout << "Tuples:\n";
    for (auto &tuple : tuples) cout << tuple << '\n';
    cout << '\n';

    // Initializing the algorithm
    GA1 ga1(tuples, workloads, out_periods);
    for (int i = 0; i < ga1.pop_size; ++i) {
        cout << "Chromossome " << i << ":\n";
        for (int j = 0; j < ga1.periods_size; ++j)
            cout << "period " << j << ": " << ga1.population[i][j] << '\n';
        cout << '\n';
    }

    // Refining the initial population
    ga1.start();

    return 0;
}
