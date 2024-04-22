//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_RECT_H
#define ARCHIVATOR_RECT_H

#include <cstdlib>

struct Rect {
    size_t a;
    size_t b;
    size_t c;
    size_t d;

    explicit Rect(cv::Rect rect) {
        a = rect.x;
        b = rect.y;
        c = rect.x + rect.width;
        d = rect.y + rect.height;
    }
};
#endif //ARCHIVATOR_RECT_H
