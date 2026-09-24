#pragma once

#include <string>
#include <vector>

namespace editor {

    struct Shader {
        std::string key, vertex, fragment;
        int depth;
    };

    struct Animator {
        std::string textureKey;
        std::vector<Sprite::Anim> animations;
    };
}