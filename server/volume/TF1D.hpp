#ifndef TF1D_HPP
#define TF1D_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#include "../../common/Vector.hpp"

#define COLOR_MAP_SIZE 256
struct TF_key {
    float position;
    Vector4f color;
};

class TF1D {
public:
    TF1D() {
        std::vector<TF_key> tf_keys;
        tf_keys.push_back({0, {0, 0, 0, 0}});
        tf_keys.push_back({0.1, {0, 0, 0, 0}});
        tf_keys.push_back({1, {1, 1, 1, 1}});
        generateColorMap(tf_keys, {0, 1});
    }

    void load(const string &tf1d_str) {
        std::vector<TF_key> tf_keys;
        int key_nums;   //键的数量
        Vector2f range; //键的范围

        istringstream input(tf1d_str);
        input >> key_nums >> range.x >> range.y;
        float r, g, b, a;
        for (int i = 0; i < key_nums; i++) {
            TF_key tf_key;
            input >> tf_key.position >> r >> g >> b >> a;
            tf_key.color.x = r;
            tf_key.color.y = g;
            tf_key.color.z = b;
            tf_key.color.w = a;
            tf_keys.push_back(tf_key);
        }
        generateColorMap(tf_keys, range);
    }

    inline Vector4f sample(float position) {
        assert(position < COLOR_MAP_SIZE);
        auto posIndex = (int) position;                //取位置的整数部分
        float posFraction = position - posIndex;       //取位置的小数部分
        int nextIndex = (posIndex < COLOR_MAP_SIZE - 1) ? posIndex : (COLOR_MAP_SIZE - 1);
        return color_map[posIndex] * (1 - posFraction) + color_map[nextIndex] * posFraction;
    }

    inline const Vector4f &sampleSimple(uchar position) {
        return color_map[position];
    }

    inline const Vector4f *getColorMap() {
        return color_map;
    }

private:
    Vector4f color_map[COLOR_MAP_SIZE];

    inline void generateColorMap(const vector<TF_key> &tf_keys, const Vector2f &range) {
        for (int i = 0; i < tf_keys.size() - 1; i++) {
            auto front = (int) ((tf_keys[i].position - range.x) / (range.y - range.x) * COLOR_MAP_SIZE);
            auto rear = (int) ((tf_keys[i + 1].position - range.x) / (range.y - range.x) * COLOR_MAP_SIZE);
            for (int idx = front; idx < rear; idx++) {
                float ratio_front = (float) (idx - front) / (rear - front);
                color_map[idx] = tf_keys[i].color * (1 - ratio_front) + tf_keys[i + 1].color * ratio_front;
            }
        }
    }
};

#endif //TF1D_HPP
