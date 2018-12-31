#ifndef MMU_HPP
#define MMU_HPP

#include <cstdlib>
#include <string>

using namespace std;

#include "../../common/Vector.hpp"
#include "Block.hpp"

struct Entry {
    bool valid = false;
    int blk_idx;
};

class MMU {
public:
    MMU(const Vector3i &_dims, const Vector3i &_brick_size, float _brick_num_ratio, const string &path)
            : dims(_dims), brick_size(_brick_size) {

        brick_dims.x = (int) ceil((float) _dims.x / brick_size.x);
        brick_dims.y = (int) ceil((float) _dims.y / brick_size.y);
        brick_dims.z = (int) ceil((float) _dims.z / brick_size.z);

        int N = brick_dims.x * brick_dims.y * brick_dims.z;
        entries = new Entry[N];

        blk_nums = (int) (N * _brick_num_ratio);
        blocks = new Block[blk_nums];

        N = (brick_size.x + 1) * (brick_size.y + 1) * (brick_size.z + 1);
        data = new uchar[N * blk_nums];
        for (int i = 0; i < blk_nums; i++) {
            int offset = N * i;
            blocks[i].setMem(data + offset);
        }

        volume_path_blk = get_blk_filename(brick_size, path);

        N = brick_dims.x * brick_dims.y * brick_dims.z * 256;
        cnt_buf = new int[N];
        string cnt_path = get_cnt_filename(brick_size, path);
        fin.open(cnt_path, ios::binary | ios::in);
        if (!fin.is_open()) {
            cout << "Can not open count file!" << endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char *>(cnt_buf), N * sizeof(int));
        fin.close();

        fin.open(volume_path_blk, ios::binary | ios::in);
        if (!fin.is_open()) {
            cout << "Can not open volume blocked file!" << endl;
            exit(1);
        }

        pthread_mutex_init(&mutex, nullptr);
    }

    ~MMU() {
        pthread_mutex_destroy(&mutex);
        delete[] cnt_buf;
        delete[] entries;
        delete[] blocks;
        delete[] data;
    }

    void loadBlock(int x, int y, int z, int blk_idx) {
        assert(x < brick_dims.x);
        assert(y < brick_dims.y);
        assert(z < brick_dims.z);

        int N = (brick_size.x + 1) * (brick_size.y + 1) * (brick_size.z + 1);

        int file_offset = ((z * brick_dims.y + y) * brick_dims.x + x) * N;

        blocks[blk_idx].load(fin, file_offset, N);
    }

    Block *getBlock(int x, int y, int z, TF1D *tf1d = nullptr) {
        if (x >= brick_dims.x) return nullptr;
        if (y >= brick_dims.y) return nullptr;
        if (z >= brick_dims.z) return nullptr;

        //如果块为空，则不将其加载到内存中
        if (tf1d != nullptr) {
            int cnt_base = ((z * brick_dims.y + y) * brick_dims.x + x) * 256;
            int idx;
            for (idx = 0; idx < 256; idx++) {
                if (cnt_buf[cnt_base + idx] != 0 && tf1d->sampleSimple(idx).w != 0) {
                    break;
                }
            }
            if (idx == 256) {
                return nullptr;
            }
        }

        int blk_idx;
        int entry_idx = (z * brick_dims.y + y) * brick_dims.x + x;

        if (entries[entry_idx].valid) {
            blk_idx = entries[entry_idx].blk_idx;
            blocks[blk_idx].used = true;
            return &blocks[blk_idx];
        }

        pthread_mutex_lock(&mutex);
        if (blk_used < blk_nums) {
            blk_idx = blk_used;
            blk_used++;
            clock_pos = blk_used;
        } else {
            blk_idx = clock_pos % blk_nums;
            while (blocks[blk_idx].used) {
                blocks[blk_idx].used = false;
                clock_pos++;
                blk_idx = clock_pos % blk_nums;
            }
            int old_idx = blocks[blk_idx].entry_idx;
            entries[old_idx].valid = false;
        }
        assert(blk_idx < blk_nums);

        loadBlock(x, y, z, blk_idx);

        entries[entry_idx].valid = true;
        entries[entry_idx].blk_idx = blk_idx;
        blocks[blk_idx].entry_idx = entry_idx;

        pthread_mutex_unlock(&mutex);

        return &blocks[blk_idx];
    }

    uchar *getDataBuf() {
        return data;
    }

private:
    string volume_path_blk;
    Vector3i dims, brick_dims, brick_size;
    Entry *entries;

    int blk_nums, clock_pos = 0, blk_used = 0;
    Block *blocks;              //缓冲区
    int *cnt_buf;
    uchar *data;

    pthread_mutex_t mutex;

    ifstream fin;
};

#endif //MMU_HPP
