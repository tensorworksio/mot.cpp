#pragma once

#include <cmath>
#include <vector>
#include <numeric>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace vector_ops
{

    // Element-wise addition of two vectors
    template <typename T>
    inline std::vector<T> add(const std::vector<T> &a, const std::vector<T> &b)
    {
        if (a.size() != b.size())
        {
            throw std::invalid_argument("Vectors must be the same size");
        }

        std::vector<T> result(a.size());
        std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::plus<>());
        return result;
    }

    // Scalar add
    template <typename T>
    inline std::vector<T> add(const std::vector<T> &vec, T scalar)
    {
        std::vector<T> result(vec.size());
        std::transform(vec.begin(), vec.end(), result.begin(), [scalar](T x)
                       { return x + scalar; });
        return result;
    }

    // Element-wise multiplication of two vectors
    template <typename T>
    inline std::vector<T> mul(const std::vector<T> &a, const std::vector<T> &b)
    {
        if (a.size() != b.size())
        {
            throw std::invalid_argument("Vectors must be the same size");
        }

        std::vector<T> result(a.size());
        std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::multiplies<>());
        return result;
    }

    // Scalar multiplication
    template <typename T>
    inline std::vector<T> mul(const std::vector<T> &vec, T scalar)
    {
        std::vector<T> result(vec.size());
        std::transform(vec.begin(), vec.end(), result.begin(), [scalar](T x)
                       { return x * scalar; });
        return result;
    }

    // Dot product of two vectors
    template <typename T>
    inline T dot(const std::vector<T> &a, const std::vector<T> &b)
    {
        if (a.size() != b.size())
        {
            throw std::invalid_argument("Vectors must be the same size");
        }

        return std::inner_product(a.begin(), a.end(), b.begin(), T(0));
    }

    // Normalize vector
    template <typename T>
    inline std::vector<T> normalize(const std::vector<T> &vec)
    {
        std::vector<T> result(vec.size());
        T norm = std::sqrt(dot(vec, vec));
        result = mul(vec, T(1) / norm);
        return result;
    }

    // Compose 2 vectors with a weighted average
    template <typename T>
    inline std::vector<T> compose(const std::vector<T> &a, const std::vector<T> &b, T alpha)
    {
        if (a.size() != b.size())
        {
            throw std::invalid_argument("Vectors must be the same size");
        }
        const T alpha_complement = T(1) - alpha;
        std::vector<T> result(a.size());
        std::transform(a.begin(), a.end(), b.begin(), result.begin(), [alpha, alpha_complement](T x, T y)
                       { return alpha * x + alpha_complement * y; });
        return result;
    }

    // Sum vector
    template <typename T>
    inline T sum(const std::vector<T> &vec)
    {
        return std::accumulate(vec.begin(), vec.end(), T(0));
    }

    // Mean vector
    template <typename T>
    inline T mean(const std::vector<T> &vec)
    {
        if (vec.empty())
        {
            return T(0);
        }
        T size = static_cast<T>(vec.size());
        return sum(vec) / size;
    }

    // Max vector
    template <typename T>
    inline T max(const std::vector<T> &vec)
    {
        return *std::max_element(vec.begin(), vec.end());
    }

    // Exp vector
    template <typename T>
    inline std::vector<T> exp(const std::vector<T> &vec)
    {
        std::vector<T> result(vec.size());
        std::transform(vec.begin(), vec.end(), result.begin(), [](T x)
                       { return std::exp(x); });
        return result;
    }

    // Slice vector
    template <typename T>
    inline std::vector<T> slice(const std::vector<T> &vec, int start, int end)
    {
        auto first = vec.begin() + start;
        auto last = vec.begin() + end;
        std::vector<T> sliced(first, last);
        return sliced;
    }

} // namespace vector_ops