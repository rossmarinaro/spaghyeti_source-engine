#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/game.h"

class GAME3 : public Game {
     public:
         GAME3() { name = GAME3; }
         void Preload() override;
         void Run(Camera* camera) override;
         void Update(Inputs* inputs, Camera* camera) override;
};