#if DEVELOPMENT == 1

#pragma once

#include "./entity.h"

class DisplayInfo {

    public: 

        DisplayInfo();
        void Update(void* _context);

    private:
        
        std::unique_ptr<Text> fps_text;
        std::unique_ptr<Text> cam_text;
        std::unique_ptr<Text> entity_text;
};

#endif