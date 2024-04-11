#pragma once

#include "./renderer.h"

//system inputs
class Inputs {

    public:

        int numInputs;

        bool cursorReset,
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
        void CreateCursor();
        void RenderCursor();
        void ToggleVirtualButtonVisibility(bool visibility);
        void ProcessInput(GLFWwindow* window);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
        static void input_callback(GLFWwindow* window, int input, int action, int mods);

        Inputs();
       ~Inputs() = default;

    private:

        float cursorX, cursorY;

        bool initVirtualControls = false;

        void CheckOverlap();
        void ResetControls();
        void SetGamepadInputs(unsigned int joystick);
        void SetKeyInputs(bool boolean, int key, GLFWwindow* window);

};
    
 

