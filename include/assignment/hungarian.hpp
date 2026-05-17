#pragma once
#include <assignment/lap.h>
#include <opencv2/core.hpp>
#include <vector>

namespace hungarian {

// Wraps lapjv's min-cost lap() to solve max-cost assignment.
// cost must be a square CV_32F matrix. Returns assignment[i] = j.
inline std::vector<long> max_cost_assignment(const cv::Mat_<float>& cost)
{
    const int n = cost.rows;
    if (n == 0)
        return {};

    // Negate: lapjv minimises, we want to maximise.
    std::vector<float> neg(n * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            neg[i * n + j] = -cost(i, j);

    std::vector<int> rowsol(n), colsol(n);
    std::vector<float> u(n), v(n);
    lap<false, false>(n, neg.data(), rowsol.data(), colsol.data(), u.data(), v.data());

    std::vector<long> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = static_cast<long>(rowsol[i]);
    return result;
}

} // namespace hungarian
