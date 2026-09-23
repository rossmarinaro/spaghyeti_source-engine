#pragma once

#include <string>

namespace editor {

    struct Shader {
        std::string key, vertex, fragment;
        int depth;
    };

    //todo: update animations, spritesheets to structs from std containers
}