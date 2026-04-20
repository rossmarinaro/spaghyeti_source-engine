#pragma once

#include "./math.h"


namespace /* SPAGHYETI_CORE */ System {

    //main window
    class Window {

        public: 
      
            static inline float s_width, 
                                s_height, 
                                s_scaleWidth = 1480.0f,/* 1500.0f, */
                                s_scaleHeight = 860.0f;/* 900.0f */;

            static inline const char* s_glsl_version;

            static void Init();
            static const Math::Vector2 GetNDCToPixel(float x, float y);
            static const Math::Vector2 GetPixelToNDC(float x, float y);
     
    };

}
 