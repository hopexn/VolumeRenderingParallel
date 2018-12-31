#ifndef BOX_H
#define BOX_H
//C头文件
#include <cfloat>
//C++头文件
#include <vector>

using namespace std;

//用户头文件
#include "Vector.hpp"

class BBox {
public:
    Vector3f bmin, bmax;
    Vector3f bsize;

    BBox(const Vector3f &_bmin, const Vector3f &_bmax) {
        assert(_bmax.x >= _bmin.x);
        assert(_bmax.y >= _bmin.y);
        assert(_bmax.z >= _bmin.z);
        bmin = _bmin;
        bmax = _bmax;
        bsize = bmax - bmin;
    }

    Vector2f getRayDistance(const Vector3f &camera_position, const Vector3f &ray_direction) {
        Vector3f tmin(FLT_MIN, FLT_MIN, FLT_MIN), tmax(FLT_MAX, FLT_MAX, FLT_MAX);

        if (ray_direction.x != 0) {
            if (ray_direction.x > 0) {
                tmin.x = (bmin.x - camera_position.x) / ray_direction.x;
                tmax.x = (bmax.x - camera_position.x) / ray_direction.x;
            } else {
                tmax.x = (bmin.x - camera_position.x) / ray_direction.x;
                tmin.x = (bmax.x - camera_position.x) / ray_direction.x;
            }
        }

        if (ray_direction.y != 0) {
            if (ray_direction.y > 0) {
                tmin.y = (bmin.y - camera_position.y) / ray_direction.y;
                tmax.y = (bmax.y - camera_position.y) / ray_direction.y;
            } else {
                tmax.y = (bmin.y - camera_position.y) / ray_direction.y;
                tmin.y = (bmax.y - camera_position.y) / ray_direction.y;
            }
        }

        if (ray_direction.z != 0) {
            if (ray_direction.z > 0) {
                tmin.z = (bmin.z - camera_position.z) / ray_direction.z;
                tmax.z = (bmax.z - camera_position.z) / ray_direction.z;
            } else {
                tmax.z = (bmin.z - camera_position.z) / ray_direction.z;
                tmin.z = (bmax.z - camera_position.z) / ray_direction.z;
            }
        }

        Vector2f res;
        res.x = max(tmin.x, max(tmin.y, tmin.z));
        res.y = min(tmax.x, min(tmax.y, tmax.z));

        if (res.x > res.y) {
            res.x = 0;
            res.y = 0;
        }
        return res;
    }

    bool intersects(const Vector3f &camera_position, const Vector3f &ray_direction) {
        float t;
        Vector3f pointOnPlane;
        if (ray_direction.x != 0) {
            if (ray_direction.x > 0) {
                t = (bmin.x - camera_position.x) / ray_direction.x;
            } else {
                t = (bmax.x - camera_position.x) / ray_direction.x;
            }
            if (t >= 0) {
                pointOnPlane = camera_position + ray_direction * t;
                if (pointOnPlane.y >= bmin.y && pointOnPlane.y <= bmax.y &&
                    pointOnPlane.z >= bmin.z && pointOnPlane.z <= bmax.z) {
                    return true;
                }
            }
        }

        if (ray_direction.y != 0) {
            if (ray_direction.y > 0) {
                t = (bmin.y - camera_position.y) / ray_direction.y;
            } else {
                t = (bmax.y - camera_position.y) / ray_direction.y;
            }
            if (t >= 0) {
                pointOnPlane = camera_position + ray_direction * t;
                if (pointOnPlane.x >= bmin.x && pointOnPlane.x <= bmax.x &&
                    pointOnPlane.z >= bmin.z && pointOnPlane.z <= bmax.z) {
                    return true;
                }
            }
        }

        if (ray_direction.z != 0) {
            if (ray_direction.z > 0) {
                t = (bmin.z - camera_position.z) / ray_direction.z;
            } else {
                t = (bmax.z - camera_position.z) / ray_direction.z;
            }
            if (t >= 0) {
                pointOnPlane = camera_position + ray_direction * t;
                if (pointOnPlane.x >= bmin.x && pointOnPlane.x <= bmax.x &&
                    pointOnPlane.y >= bmin.y && pointOnPlane.y <= bmax.y) {
                    return true;
                }
            }
        }

        return false;
    }
};


#endif //BOX_H
