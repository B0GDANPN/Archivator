
#include <vector>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
pixel_value* IFSTransform::down_sample(const pixel_value* src, int src_width, int start_x, int start_y, int target_size) {
  auto *dest = new pixel_value[target_size * target_size];
  int dest_x = 0;
  int dest_y = 0;

  for (int y = start_y; y < start_y + target_size * 2; y += 2) {
    for (int x = start_x; x < start_x + target_size * 2; x += 2) {
      // Perform simple 2x2 average
      int pixel = 0;
      pixel += src[y * src_width + x];
      pixel += src[y * src_width + (x + 1)];
      pixel += src[(y + 1) * src_width + x];
      pixel += src[(y + 1) * src_width + (x + 1)];
      pixel /= 4;

      dest[dest_y * target_size + dest_x] = pixel;
      dest_x++;
    }
    dest_y++;
    dest_x = 0;
  }

  return dest;
}
bool IFSTransform::is_positive_x() const{
  return (
          symmetry == SYM_NONE ||
          symmetry == SYM_R90 ||
          symmetry == SYM_VFLIP ||
          symmetry == SYM_RDFLIP
  );
}



bool IFSTransform::is_positive_y() const {
  return (
          symmetry == SYM_NONE ||
          symmetry == SYM_R270 ||
          symmetry == SYM_HFLIP ||
          symmetry == SYM_RDFLIP
  );
}

bool IFSTransform::is_scanline_order() const {
  return (
          symmetry == SYM_NONE ||
          symmetry == SYM_R180 ||
          symmetry == SYM_HFLIP ||
          symmetry == SYM_VFLIP
  );
}



void IFSTransform::execute(const pixel_value *src, int src_width, pixel_value *dest, int dest_width,
    bool downsampled) const {
    int from_x = this->from_x / 2;
    int from_y = this->from_y / 2;
    int d_x = 1;
    int d_y = 1;
    bool in_order = is_scanline_order();

    if (!downsampled) {
        pixel_value *new_src = down_sample(src, src_width, this->from_x, this->from_y, size);
        src = new_src;
        src_width = size;
        from_x = from_y = 0;
    }

    if (!is_positive_x()) {
        from_x += size - 1;
        d_x = -1;
    }

    if (!is_positive_y()) {
        from_y += size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;

    for (size_t to_y = this->to_y; to_y < (this->to_y + size); to_y++) {
        for (size_t to_x = this->to_x; to_x < (this->to_x + size); to_x++) {


            int pixel = src[from_y * src_width + from_x];
            pixel = static_cast<int>(scale * pixel) + offset;

            if (pixel < 0)
                pixel = 0;
            if (pixel > 255)
                pixel = 255;

            dest[to_y * dest_width + to_x] = pixel;

            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }

        if (in_order) {
            from_x = start_x;
            from_y += d_y;
        } else {
            from_y = start_y;
            from_x += d_x;
        }
    }

    if (!downsampled) {
        delete[]src;
        src = nullptr;
    }
}



Transforms::~Transforms(){
        for (int i = 0; i < channels; i++) {
            for (auto &j: ch[i])
                delete j;

            ch[i].clear();
        }
    }
int Transforms::get_size() const{
        int res=sizeof(channels)+(ch[0].size()+ch[1].size()+ch[2].size())*sizeof(IFSTransform);
        return res;
    }