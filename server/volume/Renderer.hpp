#ifndef VOLUMERENDERER_H
#define VOLUMERENDERER_H

#include <fstream>

using namespace std;

#include "../../common/util.h"
#include "../../common/Vector.hpp"
#include "../../common/Image.hpp"
#include "../../common/avx_func.h"

#include "../../common/Camera.hpp"
#include "Volume.hpp"
#include <omp.h>

#define MAX_PT_NUMS 10000

struct PointList {
    inline void push_back(float point) {
        if (size < MAX_PT_NUMS) {
            points[size++] = point;
        }
    }

    inline void clear() {
        size = 0;
    }

    inline float get(int idx) {
        return points[idx];
    }

    int size = 0;
    float points[MAX_PT_NUMS];
};

class Renderer {
private:
    Image *image;

    int thread_nums = 4;

    float sampling_rate = 1.0;

    SegList *seg_lists;

    PointList *point_lists;

public:
    Renderer(int _width, int _height, int _thread_nums) {
        image = new Image(_width, _height);
        thread_nums = _thread_nums;
        seg_lists = new SegList[thread_nums];
        point_lists = new PointList[thread_nums];
    }

    ~Renderer() {
        delete image;
        delete seg_lists;
        delete point_lists;
    }

    void renderImage(Volume *volume, Camera *camera, Pixel *buf, int comm_size, int my_rank) {
        assert(volume);
        assert(camera);

        const int width = image->getWidth();
        const int height = image->getHeight();
        const float aspect = (float) width / height;

        const Vector3f cam_pos = camera->getPosition();
        const Vector3f cam_dir = camera->getLookDirection();
        const Vector3f cam_right = camera->getRightDirection();
        const Vector3f cam_up = camera->getUpDirection();
        const float cam_plane_distance = camera->getPlaneDistance();

        /* 计算体素三个方向上最大的维度，用于计算采样步长 */
        Vector3i dims = volume->getDims();

        const float step_len = 1.f / (sampling_rate * dims.norm());

        int line_nums = (int) ceil((float) height / comm_size);
        int t_line_nums = (int) ceil((float) line_nums / thread_nums);

        Vector3i brick_dims = volume->brick_dims;
        int N = brick_dims.x * brick_dims.y * brick_dims.z;
        auto offsets = new int[N];
        uchar *buf_base = volume->getMmu()->getDataBuf();

        for (int k = 0; k < brick_dims.z; k++) {
            for (int j = 0; j < brick_dims.y; j++) {
                for (int i = 0; i < brick_dims.x; i++) {
                    int index = ((k * brick_dims.y + j) * brick_dims.x + i);
                    Block *block = volume->getMmu()->getBlock(i, j, k);
                    offsets[index] = (block == nullptr) ? (int) (block->data - buf_base) : -1;
                }
            }
        }

#pragma omp parallel
        {
            int t_id = omp_get_thread_num();
            SegList *seg_list = &seg_lists[t_id];
            PointList *point_list = &point_lists[t_id];
            for (int i = 0; i < t_line_nums; i++) {
                int row = my_rank + (t_id * t_line_nums + i) * comm_size;
                if (row >= height) break;

                for (int col = 0; col < width; col++) {
                    float u = -0.5f + (col + .5f) / width;
                    u *= aspect;
                    float v = -0.5f + (row + .5f) / height;
                    Vector3f ray_dir = cal_ray_dir(cam_dir, cam_up, cam_right, cam_plane_distance, u, v);
                    ray_dir = ray_dir.normalize();

                    Vector2f seg = volume->getRayDistance(cam_pos, ray_dir);

                    int idx = (row / comm_size) * width + col;
                    volume->getSegList(*seg_list, cam_pos, ray_dir);
                    point_list->clear();
                    for (int i = 0; i < seg_list->size; i++) {
                        const Seg &s = seg_list->get(i);
                        float s_beg = s.bg;
                        float s_end = s.ed;
                        s_beg = seg.x + ceil((s_beg - seg.x) / step_len) * step_len;
                        while (s_beg <= s_end) {
                            point_list->push_back(s_beg);
                            s_beg += step_len;
                        }
                    }
                }
            }
        }
    }
};

#endif //VOLUMERENDERER_H

