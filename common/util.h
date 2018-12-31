#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;


#include <cstdlib>
#include <cstdio>

#include "Vector.hpp"
#include <sys/types.h>
#include <unistd.h>

//定义uchar
typedef unsigned char uchar;

//定义无穷大
#define INFINITE 1000000

#define EPS 1e-8

//π
const float M_PI_F = 3.1415926535f;

//tf1d文件的大小不能超过这个值
const int TF1D_BUF_SIZE = 1024 * 10;
const int MSG_BUF_SIZE = 1024;

#define PORT 9523

struct vifo {
    Vector3i dims, blk_size;
    Vector3f bmin, bmax;
    float mmu_ratio;
    string volume_file;
};

//2的整数幂
inline int pow2i(int exp) {
    int res = 1;
    for (int i = 0; i < exp; i++) {
        res <<= 1;
    }
    return res;
}

inline Vector3i getNodePosition(int index) {
    Vector3i pos(0, 0, 0);
    while (index != 0) {
        index--;
        pos.x <<= 1;
        pos.y <<= 1;
        pos.z <<= 1;
        pos.x |= (index & 0b001) ? 1 : 0;
        pos.y |= (index & 0b010) ? 1 : 0;
        pos.z |= (index & 0b100) ? 1 : 0;
        index >>= 3;
    }
    return pos;
}

inline int getNodeDepth(int index) {
    int node_depth = 0;
    while (index != 0) {
        index--;
        index >>= 3;
        node_depth++;
    }
    return node_depth;
}

inline static
string get_blk_filename(const Vector3i &brick_size, const string &input_path) {
    ostringstream ss;
    ss << input_path << "_" << brick_size.x << "x" << brick_size.y << "x" << brick_size.z << ".blk";
    return ss.str();
}

inline static
string get_cnt_filename(const Vector3i &brick_size, const string &input_path) {
    ostringstream ss;
    ss << input_path << "_" << brick_size.x << "x" << brick_size.y << "x" << brick_size.z << ".cnt";
    return ss.str();
}

inline void my_fseek(FILE *fp, int out_base, int where) {
    unsigned int step = 0x7fffffff;
    if (fseek(fp, out_base, SEEK_SET)) {
        fseek(fp, 0, SEEK_SET);
        do {
            fseek(fp, step, SEEK_CUR);
            out_base -= step;
        } while (fseek(fp, out_base, SEEK_CUR));
    }
}

inline static
string volume_rearrangement(const Vector3i &dims, const Vector3i &brick_size, const string &input_path) {
    const string output_path = get_blk_filename(brick_size, input_path);

    ifstream fin;
    fin.open(output_path);
    if (fin.is_open()) {
        fin.close();
        return output_path;
    }

    fin.open(input_path, ios::in | ios::binary);
    if (!fin.is_open()) {
        cout << "Could not open input file" << endl;
        exit(0);
    }

    ofstream fout;
    fout.open(output_path, ios::binary | ios::out);
    if (!fout.is_open()) {
        cout << "Could not open output file" << endl;
        exit(0);
    }

    int N = dims.x * dims.y * dims.z;
    uchar *buf = new uchar[N];
    fin.read(reinterpret_cast<char *>(buf), N);
    fin.close();

    Vector3i brick_dims;
    brick_dims.x = (int) std::ceil((float) dims.x / brick_size.x);
    brick_dims.y = (int) std::ceil((float) dims.y / brick_size.y);
    brick_dims.z = (int) std::ceil((float) dims.z / brick_size.z);

    int brick_N = (brick_size.x + 1) * (brick_size.y + 1) * (brick_size.z + 1);
    uchar *buf_out = new uchar[brick_N];

    string cnt_path = get_cnt_filename(brick_size, input_path);
    ofstream fcnt(cnt_path, ios::binary | ios::out);
    if (!fcnt.is_open()) {
        cout << "Could not open count file" << endl;
        exit(0);
    }

    int cnt[256];
    for (int z = 0; z < brick_dims.z; z++) {
        for (int y = 0; y < brick_dims.y; y++) {
            for (int x = 0; x < brick_dims.x; x++) {
                for (int l = 0; l < 256; l++) {
                    cnt[l] = 0;
                }
                for (int w = 0; w <= brick_size.z; w++) {
                    for (int v = 0; v <= brick_size.y; v++) {
                        for (int u = 0; u <= brick_size.x; u++) {
                            int xBase = x * brick_size.x + u;
                            int yBase = y * brick_size.y + v;
                            int zBase = z * brick_size.z + w;
                            int brick_offset = ((w * (brick_size.y + 1) + v) * (brick_size.x + 1) + u);
                            if (xBase >= dims.x || yBase >= dims.y || zBase >= dims.z) {
                                buf_out[brick_offset] = 0;
                            } else {
                                int volume_offset = ((zBase * dims.y + yBase) * dims.x + xBase);
                                buf_out[brick_offset] = buf[volume_offset];
                            }
                            uchar l = buf_out[brick_offset];
                            cnt[l]++;
                        }
                    }
                }
                int out_base = ((z * brick_dims.y + y) * brick_dims.x + x) * brick_N;
                int cnt_base = ((z * brick_dims.y + y) * brick_dims.x + x) * 256 * sizeof(int);
                fout.seekp(out_base, ios::beg);
                fout.write(reinterpret_cast<const char *>(buf_out), brick_N);
                fcnt.seekp(cnt_base, ios::beg);
                fcnt.write(reinterpret_cast<const char *>(cnt), sizeof(int) * 256);
            }
        }
    }
    delete[] buf;
    delete[] buf_out;
    fcnt.close();
    return output_path;
}



#define MAX_STR_LEN 128

#define DEBUG

#endif //COMMON_H
