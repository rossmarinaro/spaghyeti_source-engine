#pragma once

#include "./vendors/glm/glm.hpp"


namespace /* SPAGHYETI_CORE */ System {

    //main window
    class Window {

        public: 
      
            static inline float s_width, 
                                s_height, 
                                s_scaleWidth = 1500.0f, 
                                s_scaleHeight = 900.0f;

            static inline const char* s_glsl_version;

            static void Init();
            static const glm::vec2 GetNDCToPixel(float x, float y);
            static const glm::vec2 GetPixelToNDC(float x, float y);
     
    };

}
