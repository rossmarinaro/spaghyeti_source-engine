#pragma once

//system inputs
class Inputs {

    public:

        int numInputs;

        bool cursorReset,
             RIGHT_CLICK,
             LEFT_CLICK,
             SPACE, 
             SHIFT, 
             ENTER,
             TAB, 
             LEFT,
             RIGHT,
             UP,
             DOWN,
             W,
             A,
             S,
             D,
             G,
             isDown = false;

        float mouseX, mouseY;

        void ShutDown();
        void ResetControls();
        void ProcessInput();
        void SetKeyInputs(bool boolean, int key, void* window_ptr);
        void SetGamepadInputs(unsigned int joystick);

        Inputs();
       ~Inputs() = default;

    private:

        bool m_initVirtualControls;
};
    
 

