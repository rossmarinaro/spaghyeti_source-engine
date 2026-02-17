#pragma once

#include <cmath>
 
namespace Math {

    struct Vector2 { float x, y; };
    struct Vector3 { float x, y, z; };
    struct Vector4 { float r, g, b, a; };
    struct Matrix4 { Vector4 a, b, c, d; };

    static const double distanceBetween(float x) {
        return sqrt(x * x);
    }

    namespace Graphics {
        struct Vertex 
        { 
            float x, y, z, 
                  scaleX, scaleY,
                  rotation,
                  u, v, 
                  texID, 
                  r, g, b, a; 

            float modelView[16];
        }; 
    }
}

