#pragma once
#include <algorithm>
#include <cstdint>
#include <utility>

template <typename T>
    requires(std::integral<T>)
static inline constexpr T div_ceil(T a, T b) {
    return a / b + (a % b != 0);
}

/**
@brief calculate the dimension of the linear transformed image
@return [width, height]
*/
template <typename T>
static inline constexpr std::pair<std::size_t, std::size_t>
get_linear_transformed_dimension(std::uint32_t width, std::uint32_t height, const T& A) {
    // calculate new size, through the 4 corners
    // 0, (H-1) -> -(H-1)/2, (H-1)/2
    // 0, (W-1) -> -(W-1)/2, (W-1)/2
    const double half_height = ((double)height - 1.0) / 2.0;  // y
    const double half_width = ((double)width - 1.0) / 2.0;    // x

    // 1    2
    // 3    4
    const double x1 = -half_width * A[0][0] + half_height * A[0][1];
    const double x2 = half_width * A[0][0] + half_height * A[0][1];
    const double x3 = -half_width * A[0][0] - half_height * A[0][1];
    const double x4 = half_width * A[0][0] - half_height * A[0][1];

    const double y1 = -half_width * A[1][0] + half_height * A[1][1];
    const double y2 = half_width * A[1][0] + half_height * A[1][1];
    const double y3 = -half_width * A[1][0] - half_height * A[1][1];
    const double y4 = half_width * A[1][0] - half_height * A[1][1];

    const double xmax = std::max(std::max(std::max(x1, x2), x3), x4);
    const double xmin = std::min(std::min(std::min(x1, x2), x3), x4);
    const double ymax = std::max(std::max(std::max(y1, y2), y3), y4);
    const double ymin = std::min(std::min(std::min(y1, y2), y3), y4);

    const std::uint32_t new_width = static_cast<std::uint32_t>(xmax - xmin + 1);
    const std::uint32_t new_height = static_cast<std::uint32_t>(ymax - ymin + 1);

    return { new_width, new_height };
}
