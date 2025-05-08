#if DEVELOPMENT == 1

#pragma once

#include "../../../build/sdk/include/entity.h"

class DisplayInfo {

    public: 

        DisplayInfo(void* _context);
        void Update(void* _context);

    private:

        double m_prev;
        int m_frames;
        
        std::unique_ptr<Text> fps_text;
        std::unique_ptr<Text> cam_text;
        std::unique_ptr<Text> entity_text;
};

#endif