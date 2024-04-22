#pragma once

#include "./renderer.h"

//system inputs
class Inputs {

    public:

        int numInputs;

        bool isDown = false,
             LEFT_CLICK,
             SPACE, 
             SHIFT, 
             ENTER,
             TAB, 
             LEFT,
             RIGHT,
             UP,
             DOWN,
             G;

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

        float m_cursorX, m_cursorY;

        bool m_cursorReset, m_initVirtualControls;

        void CheckOverlap();
        void ResetControls();
        void SetGamepadInputs(unsigned int joystick);
        void SetKeyInputs(bool boolean, int key, GLFWwindow* window);

};
    
 

