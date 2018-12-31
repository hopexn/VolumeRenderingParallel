#ifndef AVX_FUNC_H
#define AVX_FUNC_H

#include <iostream>
#include <string>
#include <sstream>
#include <immintrin.h>

using namespace std;

#include <cstdlib>
#include <cstdio>

#include "Vector.hpp"

inline static
Vector3f cal_ray_dir(const Vector3f &cam_pos, const Vector3f &up, const Vector3f &right,
                     float cam_plane_dist, float u, float v) {
    __m128 _pos, _up, _right, _tmp;

    _tmp = _mm_set_ps(0, u, u, u);
    _right = _mm_mul_ps(right.data, _tmp);

    _tmp = _mm_set_ps(0, v, v, v);
    _up = _mm_mul_ps(up.data, _tmp);

    _tmp = _mm_set_ps(0, cam_plane_dist, cam_plane_dist, cam_plane_dist);
    _pos = _mm_mul_ps(cam_pos.data, _tmp);

    _tmp = _mm_add_ps(_right, _up);
    _pos = _mm_add_ps(_pos, _tmp);

    return {_pos};
}

inline static
Vector3f cal_ray_pos(const Vector3f &cam_pos, const Vector3f &ray_dir, float dist) {
    __m128 res;
    res = _mm_set_ps(0, dist, dist, dist);
    res = _mm_mul_ps(ray_dir.data, res);
    res = _mm_add_ps(cam_pos.data, res);
    return {res};
}

inline static
float read_voxel(const u_int64_t *data, const Vector3f &x,
                 const Vector3i &dims, const Vector3f &bmin,
                 const Vector3f &bsize) {
    assert(data);
    float tmp_f;
    __m128 relative, dims_v, tmp1, tmp2, zero, one;
    __M128 index, fraction;

    zero = _mm_setzero_ps();
    tmp_f = 1;
    one = _mm_broadcast_ss(&tmp_f);

    relative = _mm_sub_ps(x.data, bmin.data);
    dims_v = _mm_set_ps(1, dims.z, dims.y, dims.x);
    tmp1 = _mm_div_ps(dims_v, bsize.data);
    relative = _mm_mul_ps(tmp1, relative);

    index.xyzw = _mm_floor_ps(relative);
    fraction.xyzw = _mm_sub_ps(relative, index.xyzw);

    __m128i tmp1_i = (__m128i) _mm_cmplt_ps(index.xyzw, zero);
    __m128i tmp2_i = (__m128i) _mm_cmpge_ps(index.xyzw, dims_v);
    __m128i one_i = _mm_set_epi32(1, 1, 1, 1);
    if (!_mm_test_all_zeros(tmp1_i, one_i) || !_mm_test_all_zeros(tmp2_i, one_i)) {
        return 0.f;
    }

    int idx = (index.z * dims.y + index.y) * dims.x + index.x;

    uchar *voxel = (uchar *) (&data[idx]);

    __m128 v128_l = _mm_set_ps(voxel[0], voxel[1], voxel[2], voxel[3]);
    __m128 v128_h = _mm_set_ps(voxel[4], voxel[5], voxel[6], voxel[7]);
    __m128 f_a, f_b, f_c;

    __M128 res;

    tmp_f = fraction.x;
    f_a = _mm_broadcast_ss(&tmp_f);
    f_b = _mm_set_ps(1 - fraction.y, 1 - fraction.y, fraction.y, fraction.y);
    f_c = _mm_set_ps(1 - fraction.z, fraction.z, 1 - fraction.z, fraction.z);

    tmp1 = _mm_mul_ps(v128_h, f_a);
    tmp2 = _mm_mul_ps(f_b, f_c);

    res.xyzw = _mm_mul_ps(tmp1, tmp2);

    f_a = _mm_sub_ps(one, f_a);

    tmp1 = _mm_mul_ps(v128_l, f_a);
    tmp2 = _mm_mul_ps(tmp1, tmp2);

    res.xyzw = _mm_add_ps(res.xyzw, tmp2);

    return res.x + res.y + res.z + res.w;
}

inline static
void traverse_node(const u_int64_t *data, const Vector4f *tf1d_map, Vector4f &rgba,
                   const Vector3f &bmin, const Vector3f &bsize, const Vector3i &dims,
                   const Vector3f &cam_pos, const Vector3f &ray_dir, const Vector3f &step,
                   float enter, float leave, int step_count
) {
    Vector3f ray_pos = cal_ray_pos(cam_pos, ray_dir, enter);

    for (int i = 0; i < step_count && rgba.w <= 0.99; i++) {
        float v = read_voxel(data, ray_pos, dims, bmin, bsize);

        const Vector4f &color_sample = tf1d_map[(int) v];

        float opacity_increment = (1 - rgba.w) * color_sample.w;
        rgba.x += color_sample.x * opacity_increment;
        rgba.y += color_sample.y * opacity_increment;
        rgba.z += color_sample.z * opacity_increment;
        rgba.w += opacity_increment;

        ray_pos += step;
    }
}

inline static
Vector4f blend(const Vector4f &rgba, float bg_f) {
    __m128 w, bg, tmp1, tmp2, one, f255;

    float tmp_f = 1;
    one = _mm_broadcast_ss(&tmp_f);

    w = _mm_broadcast_ss(&(rgba.w));
    tmp1 = _mm_mul_ps(rgba.data, w);

    w = _mm_sub_ps(one, w);
    bg = _mm_broadcast_ss(&bg_f);
    tmp2 = _mm_mul_ps(bg, w);

    tmp1 = _mm_add_ps(tmp1, tmp2);

    tmp_f = 255;
    f255 = _mm_broadcast_ss(&tmp_f);
    tmp2 = _mm_mul_ps(tmp1, f255);

    return {tmp2};
}

#endif //AVX_FUNC_H
