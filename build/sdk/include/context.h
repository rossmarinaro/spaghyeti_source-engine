#pragma once

#include <mutex>

#include "./camera.h"
#include "./inputs.h"
#include "./physics.h"
#include "./time.h"

namespace Process {

    struct Context {
        Inputs* inputs; 
        Camera* camera;
        Physics* physics;
        Time* time;

    };
}