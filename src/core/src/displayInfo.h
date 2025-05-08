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
        
        std::unique_ptr<Text> m_fps_text;
        std::unique_ptr<Text> m_cam_text;
        std::unique_ptr<Text> m_entity_text;
        std::unique_ptr<Text> m_rendered_text;
};

#endif