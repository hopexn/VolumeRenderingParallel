#ifndef IMAGE_H
#define IMAGE_H

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

extern "C" {
#include "../third_party/jpeg-9c/jpeglib.h"
}

typedef unsigned char uchar;

struct Pixel {
    uchar r, g, b;
};

class Image {
public:
    Image(int width, int height) : width(width), height(height) {
        pixels = new Pixel[width * height];
    }

    ~Image() {
        delete[] pixels;
    }

    Pixel &getPixel(int row, int col) {
        assert(row < height);
        assert(col < width);
        return pixels[row * width + col];
    }

    inline void setPixel(int row, int col, const Pixel &pixel) {
        assert(row < height);
        assert(col < width);
        pixels[row * width + col] = pixel;
    }

    inline void setPixel(int row, int col, uchar r, uchar g, uchar b) {
        assert(row < height);
        assert(col < width);
        int idx = row * width + col;
        pixels[idx].r = r;
        pixels[idx].g = g;
        pixels[idx].b = b;
    }

    inline int getWidth() { return width; }

    inline int getHeight() { return height; }

    Pixel *getPixelData() {
        return pixels;
    }

    int size() { return width * height * sizeof(Pixel); }

private:
    int width, height;
    Pixel *pixels = nullptr;
};


struct jpeg_img {
    uchar *buffer = nullptr;
    unsigned long size = 0;
};

inline static
jpeg_img write_image_mem(Pixel *pixels, int width, int height) {
    int quality = 90;
    struct jpeg_compress_struct cinfoDecodec;
    struct jpeg_error_mgr jpeg_error;
    JSAMPROW row_ptr[1];

    jpeg_img jpeg_info;

    cinfoDecodec.err = jpeg_std_error(&jpeg_error);
    jpeg_create_compress(&cinfoDecodec);
    jpeg_mem_dest(&cinfoDecodec, &(jpeg_info.buffer), &(jpeg_info.size));

    cinfoDecodec.image_width = static_cast<JDIMENSION>(width);
    cinfoDecodec.image_height = static_cast<JDIMENSION>(height);

    cinfoDecodec.input_components = 3;
    cinfoDecodec.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfoDecodec);
    jpeg_set_quality(&cinfoDecodec, quality, TRUE);
    jpeg_start_compress(&cinfoDecodec, TRUE);

    while (cinfoDecodec.next_scanline < cinfoDecodec.image_height) {
        row_ptr[0] = reinterpret_cast<JSAMPROW>(&pixels[cinfoDecodec.next_scanline * width]);
        jpeg_write_scanlines(&cinfoDecodec, row_ptr, 1);
    }
    jpeg_finish_compress(&cinfoDecodec);
    jpeg_destroy_compress(&cinfoDecodec);
    return jpeg_info;
}

inline static
void write_image_disk(Pixel *pixels, int width, int height, const string &path) {
    int quality = 90;
    struct jpeg_compress_struct cinfoDecodec;
    struct jpeg_error_mgr jpeg_error;
    JSAMPROW row_ptr[1];
    int row_stride;

    cinfoDecodec.err = jpeg_std_error(&jpeg_error);
    jpeg_create_compress(&cinfoDecodec);
    FILE *fp = fopen(path.c_str(), "wb");
    if (!fp) {
        cout << "Can not open image file!" << endl;
        exit(1);
    }
    jpeg_stdio_dest(&cinfoDecodec, fp);

    cinfoDecodec.image_width = static_cast<JDIMENSION>(width);
    cinfoDecodec.image_height = static_cast<JDIMENSION>(height);

    cinfoDecodec.input_components = 3;
    cinfoDecodec.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfoDecodec);
    jpeg_set_quality(&cinfoDecodec, quality, TRUE);
    jpeg_start_compress(&cinfoDecodec, TRUE);

    while (cinfoDecodec.next_scanline < cinfoDecodec.image_height) {
        row_ptr[0] = reinterpret_cast<JSAMPROW>(&pixels[cinfoDecodec.next_scanline * width]);
        jpeg_write_scanlines(&cinfoDecodec, row_ptr, 1);
    }
    jpeg_finish_compress(&cinfoDecodec);
    jpeg_destroy_compress(&cinfoDecodec);
    fclose(fp);
}

inline static
void write_ppm(const Pixel *pixels, int width, int height, const string &path) {
    assert(pixels);
    ofstream fout(path, ios::binary | ios::out);
    if (!fout.is_open()) {
        cout << "Write ppm failed!" << endl;
        exit(0);
    }
    ostringstream ppm_head;
    ppm_head << "P6\n" << width << " " << height << "\n255\n";
    fout << ppm_head.str();
    fout.write(reinterpret_cast<const char *>(pixels), width * height * sizeof(Pixel));
    fout << "\n";
    fout.close();
}

#endif
