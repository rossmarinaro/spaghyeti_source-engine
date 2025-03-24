#pragma once

#include <cmath>

namespace Math {

    typedef struct Vector2 { float x, y; };
    typedef struct Vector3 { float x, y, z; };
    typedef struct Vector4 { float x, y, z, w; };
    typedef struct Matrix4 { Vector4 a, b, c, d; };

    static const double distanceBetween(float x) {
        return sqrt(x * x);
    }
}