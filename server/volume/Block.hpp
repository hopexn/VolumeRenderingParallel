#ifndef BLOCK_H
#define BLOCK_H

#include <cassert>
#include <fstream>

using namespace std;

class Block {
public:
    int entry_idx;
    bool used;

    Block() {
        data = nullptr;
        used = false;
        entry_idx = -1;
    }

    ~Block() {}

    void load(ifstream &fin, int offset, int N) {
        assert(data);
        fin.seekg(offset, ios::beg);
        fin.read(reinterpret_cast<char *>(data), N);
    }

    void setMem(uchar *_data) {
        data = _data;
    }

    uchar *data;
};

#endif //BLOCK_H
