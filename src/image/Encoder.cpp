#include <sstream>
#include <image/Image.hpp>
#include <image/Encoder.hpp>

void ::Encoder::send_common_information(const CommonInformation& common_information){
        send_message("FractalAlgo{ ");
        IController::send_common_information(common_information);
        send_message("}\n");
    }

void Encoder::send_error_information(const std::string& error){
        IController::send_error_information("FractalAlgo{ " + error + "}\n");
    }

Encoder::Encoder(bool is_text_output, const std::string& output_file, std::ostringstream& ref_oss)
: IController(is_text_output, output_file,ref_oss), img(is_text_output, output_file, ref_oss){}
int Encoder::get_average_pixel(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int size){
        int top = 0;
        int bottom = (size * size);
        // Simple average of all pixels.
        for (int y = domain_y; y < domain_y + size; y++) {
            for (int x = domain_x; x < domain_x + size; x++) {
                top += domain_data[y * domain_width + x];
                if (top < 0) {
                    std::ostringstream oss;
                    oss << "Error: Accumulator rolled over averaging pixels." << '\n';
                    std::string str = oss.str();
                    send_error_information(str);
                    delete domain_data;
                    exit(-1);
                }
            }
        }
        return (top / bottom);
    }

double Encoder::get_scale_factor(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg, const pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg, int size)
{
        int top = 0;
        int bottom = 0;

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int domain = domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg;
                int range = (range_data[(range_y + y) * range_width + (range_x + x)] - range_avg);

                // According to the formula we want (R*D) / (D*D)
                top += range * domain;
                bottom += domain * domain;

                if (bottom < 0) {
                    std::ostringstream oss;
                    oss << "Error: Overflow occurred during scaling"
                        << y << " " << domain_width << " " << bottom << " " << top << '\n';
                    std::string str = oss.str();
                    send_error_information(str);
                    delete range_data;
                    delete domain_data;
                    exit(-1);
                }
            }
        }

        if (bottom == 0) {
            top = 0;
            bottom = 1;
        }

        return static_cast<double>(top) / static_cast<double>(bottom);
    };

double Encoder::get_error(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg, const pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg, int size, double scale){
        double top = 0;
        auto bottom = static_cast<double>(size * size);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg);
                int range = (range_data[(range_y + y) * range_width + (range_x + x)] - range_avg);
                int diff = static_cast<int>(scale * static_cast<double>(domain)) - range;

                // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
                top += (diff * diff);

                if (top < 0) {
                    std::ostringstream oss;
                    oss << "Error: Overflow occurred during error " << top << '\n';
                    std::string str = oss.str();
                    send_error_information(str);
                    delete range_data;
                    delete domain_data;
                    exit(-1);
                }
            }
        }

        return (top / bottom);
    }