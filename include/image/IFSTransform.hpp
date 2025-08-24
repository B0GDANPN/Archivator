#ifndef ARCHIVATOR_IFS_TRANSFORM_HPP
#define ARCHIVATOR_IFS_TRANSFORM_HPP


#include <vector>
#include <image/Image.hpp>

class IFSTransform {
public:
    enum Sym {
        SYM_NONE = 0,
        SYM_R90,
        SYM_R180,
        SYM_R270,
        SYM_HFLIP,
        SYM_VFLIP,
        SYM_RDFLIP,
    };
    // Spatial transformation
    size_t from_x;
    size_t from_y;
    size_t to_x;
    size_t to_y;
    size_t size;

    // Symmetry operation
    Sym symmetry;

    // Pixel intensity
    double scale;
    int offset;
    static pixel_value *down_sample(const pixel_value *src, int src_width,
                                  int start_x, int start_y, int target_size);



    IFSTransform(int from_x, int from_y, int to_x, int to_y, int size,
               Sym symmetry, double scale, int offset) : from_x(from_x), from_y(from_y),
                                                         to_x(to_x), to_y(to_y), size(size),
                                                         symmetry(symmetry), scale(scale),
                                                         offset(offset) {};

    ~IFSTransform() = default;

    void execute(const pixel_value *src, int src_width,
                 pixel_value *dest, int dest_width, bool downsampled) const;


private:

    bool is_scanline_order() const;

    bool is_positive_x() const;

    bool is_positive_y() const;
};

typedef std::vector<IFSTransform *> transform;
struct  Transforms {
    Transforms() {
       channels = 0;
    };

    ~Transforms();
    int get_size() const;
    transform ch[3];
    int channels;
};

#endif
