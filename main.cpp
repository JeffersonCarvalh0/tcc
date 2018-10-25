# include "knapsack.hpp"
# include "genetic1.hpp"

# include <map>

using namespace std;

int main() {
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
    int wl, subjects;
    vector<int> workloads, prefs;
    set<int> ans;

    cin >> wl >> subjects;
    workloads.resize(subjects); prefs.resize(subjects);

    for (int i = 0; i < subjects; ++i) cin >> workloads[i];
    for (int i = 0; i < subjects; ++i) cin >> prefs[i];

    ans = knapsack(subjects, wl, workloads, prefs);

    for (auto &e : ans) cout << e << ' ';
    cout << '\n';

    return 0;
}
