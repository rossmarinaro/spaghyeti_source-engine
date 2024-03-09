#pragma once

#include "./renderer.h"

//system inputs
class Inputs {

    public:

        float cursorX, cursorY;
        bool cursorReset,
             initVirtualControls = false,
             isDown = false,
             m_left_click,
             m_SPACE, 
             m_SHIFT, 
             m_ENTER,
             m_TAB, 
             m_left,
             m_right,
             m_up,
             m_down,
             m_G;


        void ShutDown();
        void ResetControls();
        void CreateCursor();
        void RenderCursor();
        void checkOverlap();
        void ToggleVirtualButtonVisibility(bool visibility);
        void setGamepadInputs(unsigned int joystick);
        void processInput(GLFWwindow* window);
        void setKeyInputs(bool boolean, int key, GLFWwindow* window);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
        static void input_callback(GLFWwindow* window, int input, int action, int mods);

        Inputs();
       ~Inputs() = default;

};
    
 

