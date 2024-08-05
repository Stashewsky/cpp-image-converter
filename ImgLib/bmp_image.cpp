#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <iostream>
using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    char b = 'B';
    char m = 'M';
    uint32_t header_data_size = 0; //indent * height
    uint32_t reserved_space = 0;
    uint32_t indent = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t info_header_size = 40;
    int32_t width = 0;
    int32_t height = 0;
    uint16_t planes_num = 1;
    uint16_t bit_per_pixel = 24;
    uint32_t compression_type = 0;
    uint32_t bytes_in_data = 0; //indent * height
    int32_t width_resolution = 11811;
    int32_t height_resolution = 11811;
    int32_t used_colors = 0;
    int32_t significant_colors = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    int colors_num = 3;
    int padding = 4;
    return padding * ((w * colors_num + 3) / padding);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image){
    ofstream out(file, ios::binary);

    int height = image.GetHeight();
    int width = image.GetWidth();
    int bmp_indent = GetBMPStride(width);

    BitmapFileHeader file_header;
    file_header.header_data_size = bmp_indent * height + file_header.indent;

    BitmapInfoHeader info_header;
    info_header.height = height;
    info_header.width = width;
    info_header.bytes_in_data = height * bmp_indent;

    out.write(reinterpret_cast<const char*>(&file_header), 14);
    out.write(reinterpret_cast<const char*>(&info_header), 40);

    std::vector<char> buffer(bmp_indent);

    for(int h = height - 1; h >= 0; h--){
        const Color* line = image.GetLine(h);

        for(int w = 0; w != width; w++){
            buffer[w * 3] = static_cast<char>(line[w].b);
            buffer[w * 3 + 1] = static_cast<char>(line[w].g);
            buffer[w * 3 + 2] = static_cast<char>(line[w].r);
        }

        out.write(buffer.data(), bmp_indent);
    }

    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file){
    ifstream input(file, ios::binary);

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    input.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    input.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    if(!input){
        std::cerr << "Read from file failed!" << endl;
        return;
    }
    if(file_header.b != 'B' || file_header.m != 'M'){
        std::cerr << "Invelid file signature!" << endl;
    }

    int width = info_header.width;
    int height = info_header.height;
    
    Image result(width, height, Color::Black());

    int bmp_indent = GetBMPStride(width);
    std::vector<char> buffer(bmp_indent);

    for(int h = height - 1; h >= 0; h--){
        input.read(buffer.data(), bmp_indent);
        Color* line = result.GetLine(h);

        for(int w = 0; w != width; w++){
            line[w].b = static_cast<byte>(buffer[3 * w]);
            line[w].g = static_cast<byte>(buffer[3 * w + 1]);
            line[w].r = static_cast<byte>(buffer[3 * w + 2]);
        }
    }
    return result;
}
}  // namespace img_lib