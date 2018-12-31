#ifndef VOLUME_H
#define VOLUME_H

#include <omp.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

#include <cstdlib>
#include <cassert>

#include "../../common/util.h"
#include "../../common/Vector.hpp"
#include "../../common/BBox.hpp"
#include "TF1D.hpp"
#include "MMU.hpp"

typedef struct OTNode *OTree;

enum OTNodeStatus {
    NONEMPTY, EMPTY
};

struct OTNode : public BBox {
    int index = 0, depth = 0;
    Vector3i position;
    OTNodeStatus status = EMPTY;

    OTNode() : BBox(Vector3f(0, 0, 0), Vector3f(0, 0, 0)) {}

    void setIndex(int _index) {
        index = _index;
        depth = getNodeDepth(_index);
        position = getNodePosition(_index);
    }

    void setBox(const Vector3f &_bmin, const Vector3f &_bmax) {
        bmin = _bmin;
        bmax = _bmax;
        bsize = bmax - bmin;
    }
};

struct Seg {
    float bg, ed;

    friend bool operator<(const Seg &lhs, const Seg &rhs) {
        return lhs.bg < rhs.bg;
    }
};

#define MAX_SEG_NUMS 100

struct SegList {

    inline const Seg &get(int idx) {
        return segs[idx];
    }

    inline void push_back(float bg, float ed) {
        if (size < MAX_SEG_NUMS) {
            segs[size].bg = bg;
            segs[size].ed = ed;
            size++;
        }
    }

    inline void clear() {
        size = 0;
    }

    inline void merge() {
        int i = 0, j = 0;
        if (size == 0) return;
        sort(segs, segs + size);
        while (j < size - 1) {
            if (segs[j + 1].bg - segs[j].ed > EPS) {
                segs[i].ed = segs[j].ed;
                i++;
                segs[i].bg = segs[j + 1].bg;
            }
            j++;
        }
        segs[i].ed = segs[j].ed;
        size = ++i;
    }

    int size;
    Seg segs[MAX_SEG_NUMS];
};

class Volume : public BBox {
public:
    Volume(const Vector3i &_dims, const Vector3i &_brick_size, const Vector3f &_bmin, const Vector3f &_bmax)
            : BBox(_bmin, _bmax) {
        assert(_dims.x > 0);
        assert(_dims.y > 0);
        assert(_dims.z > 0);

        assert(_brick_size.x > 0);
        assert(_brick_size.y > 0);
        assert(_brick_size.z > 0);

        dims = _dims;
        brick_size = _brick_size;

        brick_dims.x = (int) std::ceil((float) dims.x / brick_size.x);
        brick_dims.y = (int) std::ceil((float) dims.y / brick_size.y);
        brick_dims.z = (int) std::ceil((float) dims.z / brick_size.z);

        int brick_dims_max = brick_dims.max();

        depth = 0;
        while (pow2i(depth) < brick_dims_max) depth++;

        int num_nodes = (pow2i(3 * (depth + 1)) - 1) / 7;

        //计算每一个块包围盒的大小
        brick_box_size.x = bsize.x * brick_size.x / dims.x;
        brick_box_size.y = bsize.y * brick_size.y / dims.y;
        brick_box_size.z = bsize.z * brick_size.z / dims.z;

        Vector3f node_bmin, node_bmax;

        root = new OTNode[num_nodes];
        for (int i = num_nodes - 1; i >= 0; i--) {
            OTNode *node = &root[i];

            node->setIndex(i);

            if (node->depth == depth) {
                node_bmin.x = bmin.x + brick_box_size.x * node->position.x;
                node_bmin.y = bmin.y + brick_box_size.y * node->position.y;
                node_bmin.z = bmin.z + brick_box_size.z * node->position.z;
                node_bmax = node_bmin + brick_box_size;
            } else {
                const OTNode *fist_child = &root[(i << 3) + 1];
                const OTNode *last_child = &root[(i << 3) + 8];
                node_bmin = fist_child->bmin;
                node_bmax = last_child->bmax;
            }
            node->setBox(node_bmin, node_bmax);
        }

        tf1d = nullptr;

        int N = brick_dims.x * brick_dims.y * brick_dims.z;
        block_data = new uchar *[N];
        for (int i = 0; i < N; i++) {
            block_data[i] = nullptr;
        }
    }

    ~Volume() {
        delete block_data;
        delete[] root;
        delete tf1d;
        delete mmu;
    }

    inline void getSegList(SegList &seg_list, const Vector3f &camera_position, const Vector3f &ray_direction) {
        seg_list.clear();
        queue<OTNode *> que;
        que.push(&root[0]);
        while (!que.empty()) {
            OTNode *tmp = que.front();
            que.pop();
            if (tmp->status == EMPTY) {
                continue;     //如果块为空
            }
            if (!tmp->intersects(camera_position, ray_direction)) {
                continue;    //如果该块不相交
            }
            if (tmp->depth == depth) {
                Vector2f s = tmp->getRayDistance(camera_position, ray_direction);
                if (s.x < s.y) {
                    seg_list.push_back(s.x, s.y);
                }
            } else {
                for (int i = 0; i < 8; i++) {
                    que.push(&(root[((tmp->index) << 3) + i + 1]));
                }
            }
        }
        seg_list.merge();
    }

    inline Vector4f read(const Vector3f &pt_pos) {
        //相对体数据顶点的位置
        Vector3f relative = pt_pos - bmin;
        relative.x *= dims.x / bsize.x;
        relative.y *= dims.y / bsize.y;
        relative.z *= dims.z / bsize.z;

        int relative_idx_x = (int) relative.x;
        int relative_idx_y = (int) relative.y;
        int relative_idx_z = (int) relative.z;

        //判断是否在边界内
        if (relative_idx_x < 0 || relative_idx_x >= dims.x
            || relative_idx_y < 0 || relative_idx_y >= dims.y
            || relative_idx_z < 0 || relative_idx_z >= dims.z) {
            return {0, 0, 0, 0};
        }

        Vector3f blk_pos;
        blk_pos.x = relative.x / brick_size.x;
        blk_pos.y = relative.y / brick_size.y;
        blk_pos.z = relative.z / brick_size.z;

        Vector3i blk_idx;
        blk_idx.x = (int) blk_pos.x;
        blk_idx.y = (int) blk_pos.y;
        blk_idx.z = (int) blk_pos.z;

        int idx = (blk_idx.z * brick_dims.y + blk_idx.y) * brick_dims.x + blk_idx.x;
        uchar *data = block_data[idx];

        if (data == nullptr) {
            return {0, 0, 0, 0};
        }

        Vector3f blk_fraction;
        blk_fraction.x = blk_pos.x - blk_idx.x;
        blk_fraction.y = blk_pos.y - blk_idx.y;
        blk_fraction.z = blk_pos.z - blk_idx.z;

        blk_fraction.x *= brick_size.x;
        blk_fraction.y *= brick_size.y;
        blk_fraction.z *= brick_size.z;

        int xIndex = (int) blk_fraction.x;
        int yIndex = (int) blk_fraction.y;
        int zIndex = (int) blk_fraction.z;

        float xFraction = blk_fraction.x - xIndex;
        float yFraction = blk_fraction.y - yIndex;
        float zFraction = blk_fraction.z - zIndex;

        //计算各方向的偏移量
        int xNext = 1;
        int yNext = brick_size.x + 1;
        int zNext = (brick_size.x + 1) * (brick_size.y + 1);

        //表示正方体各顶点的采样值
        uchar f000, f001, f010, f011, f100, f101, f110, f111;
        int index = (zIndex * (brick_size.y + 1) + yIndex) * (brick_size.x + 1) + xIndex;

        f000 = data[index];
        f001 = data[index + zNext];
        f010 = data[index + yNext];
        f011 = data[index + yNext + zNext];
        f100 = data[index + xNext];
        f101 = data[index + xNext + zNext];
        f110 = data[index + xNext + yNext];
        f111 = data[index + xNext + yNext + zNext];

        float val = f000 * (1 - xFraction) * (1 - yFraction) * (1 - zFraction) +
                    f001 * (1 - xFraction) * (1 - yFraction) * zFraction +
                    f010 * (1 - xFraction) * yFraction * (1 - zFraction) +
                    f011 * (1 - xFraction) * yFraction * zFraction +
                    f100 * xFraction * (1 - yFraction) * (1 - zFraction) +
                    f101 * xFraction * (1 - yFraction) * zFraction +
                    f110 * xFraction * yFraction * (1 - zFraction) +
                    f111 * xFraction * yFraction * zFraction;

        return tf1d->sampleSimple((uchar)val);
    }

    bool checkStatus(int index) {
        assert(tf1d);
        OTNode *node = &root[index];
        node->status = EMPTY;
        if (node->depth == depth) {
            const Vector3i &pos = node->position;
            Block *block = mmu->getBlock(pos.x, pos.y, pos.z, tf1d);
            if (block == nullptr) {
                node->status = EMPTY;
                empty_cnt++;
                return false;
            } else {
                node->status = NONEMPTY;
                return true;
            }
        } else {
            for (int i = 0; i < 8; i++) {
                if (checkStatus((index << 3) + i + 1))
                    node->status = NONEMPTY;
            }
            return node->status == NONEMPTY;
        }
    }

    inline const Vector3i &getDims() const {
        return dims;
    }

    void setTf1d(TF1D *_tf1d) {
        tf1d = _tf1d;
    }

    inline TF1D *getTf1d() {
        return tf1d;
    }

    inline MMU *getMmu() {
        return mmu;
    }

    inline const Vector3i &getBrickDims() const {
        return brick_dims;
    }

    inline const Vector3i &getBrickSize() const {
        return brick_size;
    }

    void setMmu(MMU *_mmu) {
        mmu = _mmu;
        for (int k = 0; k < brick_dims.z; k++) {
            for (int j = 0; j < brick_dims.y; j++) {
                for (int i = 0; i < brick_dims.x; i++) {
                    int idx = (k * brick_dims.y + j) * brick_dims.x + i;
                    block_data[idx] = mmu->getBlock(i, j, k)->data;
                }
            }
        }
    }

    inline void show_empty_block_nums() {
        cout << "Empty block nums:" << empty_cnt << endl;
    }

    inline uchar **getBlockData() const {
        return block_data;
    }

    Vector3i dims;
    Vector3i brick_dims, brick_size;
    Vector3f brick_box_size;

private:
    OTree root;
    int depth;

    MMU *mmu = nullptr;
    TF1D *tf1d = nullptr;

    uchar **block_data;

    int empty_cnt = 0;
};

#endif //VOLUME_H
