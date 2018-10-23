#ifndef KNAPSACK_HPP
#define KNAPSACK_HPP

# include <iostream>
# include <vector>
# include <set>

std::set<int> knapsack(int subjects, int wl, std::vector<int> &workloads, std::vector<int> &prefs) {
    std::set<int> ans;
    std::vector<std::vector<int>> dp(subjects + 1, std::vector<int>(wl + 1));

    for (int i = 1; i <= subjects; ++i) {
        for (int j = 1; j <= wl; ++j) {
            if (workloads[i - 1] > j) dp[i][j] = dp[i - 1][j];
            else dp[i][j] = std::max(dp[i - 1][j - workloads[i - 1]] + prefs[i - 1], dp[i - 1][j]);
        }
    }

    int i = subjects, j = wl;
    while (j > 0 && i > 0) {
        if (dp[i][j] != dp[i - 1][j]) {
            ans.insert(i);
            j -= workloads[i - 1]; --i;
        } else --i;
    }

    return ans;
}

#endif /* end of include guard: KNAPSACK_HPP */
