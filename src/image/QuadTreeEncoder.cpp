
#include <vector>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/Encoder.hpp>
#include <image/QuadTreeEncoder.hpp>

Transforms* QuadTreeEncoder::encode(Image* source) {
  auto *transforms = new Transforms;
  img.width = source->width;
  img.height = source->height;
  img.channels = source->channels;
  transforms->channels = img.channels;

  for (int channel = 1; channel <= img.channels; channel++) {
    // Load image into a local copy
    img.image_data1 = new pixel_value[img.width * img.height];
    source->get_channel_data(channel, img.image_data1, img.width * img.height);


    // Make second channel the downsampled version of the image.
    img.image_data2 = IFSTransform::down_sample(img.image_data1, img.width, 0, 0, img.width / 2);


    // Go through all the range blocks
    for (int y = 0; y < img.height; y += BUFFER_SIZE) {
      for (int x = 0; x < img.width; x += BUFFER_SIZE) {
        find_matches_for(transforms->ch[channel - 1], x, y, BUFFER_SIZE);
      }
    }

    delete[]img.image_data2;
    img.image_data2 = nullptr;
    delete[]img.image_data1;
    img.image_data1 = nullptr;
  }

  return transforms;
}
void QuadTreeEncoder::find_matches_for(transform& transforms, int to_x, int to_y, int block_size){
        int best_x = 0;
        int best_y = 0;
        int best_offset = 0;
        IFSTransform::Sym best_symmetry = IFSTransform::SYM_NONE;
        double best_scale = 0;
        double best_error = 1e9;

        auto *buffer = new pixel_value[block_size * block_size];

        // Get average pixel for the range block
        int range_avg = get_average_pixel(img.image_data1, img.width, to_x, to_y, block_size);

        // Go through all the downsampled domain blocks
        for (int y = 0; y < img.height; y += block_size * 2) {
            for (int x = 0; x < img.width; x += block_size * 2) {
                auto symmetry_enum = IFSTransform::SYM_NONE;
                auto *ifs = new IFSTransform(x, y, 0, 0, block_size, symmetry_enum, 1.0, 0);
                ifs->execute(img.image_data2, img.width / 2, buffer, block_size, true);

                // Get average pixel for the downsampled domain block
                int domain_avg = get_average_pixel(buffer, block_size, 0, 0, block_size);

                // Get scale and offset
                double scale = get_scale_factor(img.image_data1, img.width, to_x, to_y, domain_avg,
                                              buffer, block_size, 0, 0, range_avg, block_size);
                int offset = static_cast<int>(range_avg - scale * static_cast<double>(domain_avg));

                // Get error and compare to best error so far
                double error = get_error(buffer, block_size, 0, 0, domain_avg,
                                        img.image_data1, img.width, to_x, to_y, range_avg, block_size, scale);

                if (error < best_error) {
                    best_error = error;
                    best_x = x;
                    best_y = y;
                    best_symmetry = symmetry_enum;
                    best_scale = scale;
                    best_offset = offset;
                }

                delete ifs;
            }
        }

        delete[]buffer;

        if (block_size > 2 && best_error >= quality_) {
            // Recurse into the four corners of the current block.
            block_size /= 2;
            find_matches_for(transforms, to_x, to_y, block_size);
            find_matches_for(transforms, to_x + block_size, to_y, block_size);
            find_matches_for(transforms, to_x, to_y + block_size, block_size);
            find_matches_for(transforms, to_x + block_size, to_y + block_size, block_size);
        } else {
            transforms.push_back(
                    new IFSTransform(
                            best_x, best_y,
                            to_x, to_y,
                            block_size,
                            best_symmetry,
                            best_scale,
                            best_offset
                    )
            );
        }
    };
