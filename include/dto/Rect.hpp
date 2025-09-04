//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_RECT_HPP
#define ARCHIVATOR_RECT_HPP
#include <opencv2/core/types.hpp>
/**
 * @brief Represents a rectangle region defined by two corner points.
 *
 * Used primarily to store image subregion coordinates (e.g., for fractal compression domain/range blocks).
 * The rectangle is defined by its top-left corner `(a, b)` and bottom-right corner `(c, d)`.
 */
struct Rect {
    /// X-coordinate of the top-left corner.
    size_t a;
    /// Y-coordinate of the top-left corner.
    size_t b;
    /// X-coordinate of the bottom-right corner.
    size_t c;
    /// Y-coordinate of the bottom-right corner.
    size_t d;
    /**
     * @brief Constructs a Rect from an OpenCV rectangle.
     * @param rect An OpenCV `cv::Rect` object (with origin and size).
     *
     * Initializes this Rect so that `(a, b)` is the top-left and `(c, d)` is the bottom-right corner of `rect`.
     */
    explicit Rect(const cv::Rect rect) :a(rect.x),b(rect.y),c(rect.x+rect.width),d(rect.y+rect.height){}
};
#endif ARCHIVATOR_RECT_HPP
