#include "../../../build/sdk/include/app.h"

using namespace System;


//--------------------------------- native input callback to browser

//mouse

#ifdef __EMSCRIPTEN__

    EM_BOOL web_mouse_callback(int eventType, const EmscriptenMouseEvent* event, void* pUserData)
    {

        if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (event->movementX != 0 || event->movementY != 0)) 
        {
            auto position = Window::GetNDCToPixel(event->targetX, event->targetY);

            Application::game->inputs->mouseX = position.x;
            Application::game->inputs->mouseY = position.y;

            Inputs::cursor_callback(Window::s_instance, event->targetX, event->targetY);
        }

        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            Application::game->inputs->numInputs++; 
            Inputs::input_callback(Window::s_instance, 1, 1, 0);
        }

        if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
            Application::game->inputs->numInputs--;
            Inputs::input_callback(Window::s_instance, 0, 0, 0);
        }

        if (eventType == EMSCRIPTEN_EVENT_CLICK)
            Application::game->inputs->LEFT_CLICK = true;

        (void)eventType;
        (void)pUserData;

        return EM_FALSE;
    }

//touch


    EM_BOOL web_touch_callback(int eventType, const EmscriptenTouchEvent* event, void* pUserData)
    {

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
        {

            Application::game->inputs->LEFT_CLICK = true;

            Application::game->inputs->cursorReset = false; 

            for (int i = 0; i < event->numTouches; ++i)
            {
                const EmscriptenTouchPoint* touch = &event->touches[i];

                auto position = Window::GetNDCToPixel(touch->targetX, touch->targetY);
            
                Application::game->inputs->mouseX = position.x;
                Application::game->inputs->mouseY = position.y;

                Inputs::cursor_callback(Window::s_instance, touch->targetX, touch->targetY);
                Inputs::input_callback(Window::s_instance, 1, 1, 0);

                Application::game->inputs->numInputs++;
            }
        }

        else
        {
            Application::game->inputs->cursorReset = true;
            Inputs::input_callback(Window::s_instance, 0, 0, 0);

            Application::game->inputs->numInputs--;
        }

        (void)eventType;
        (void)pUserData;

        return EM_FALSE;
    }

#endif


//----------------------------------------

Inputs::Inputs():
    cursorReset(false), 
    m_initVirtualControls(false)
{

    numInputs = 0;

    ResetControls();

    //browser glue input callbacks

    #ifdef __EMSCRIPTEN__

       if (Application::isMobile)
       {
            emscripten_set_touchstart_callback("canvas", 0, 1, web_touch_callback);
            emscripten_set_touchend_callback("canvas", 0, 1, web_touch_callback);
            emscripten_set_touchmove_callback("canvas", 0, 1, web_touch_callback);
       }
        
       else
       {
            emscripten_set_click_callback("canvas", 0, 1, web_mouse_callback);
            emscripten_set_mouseup_callback("canvas", 0, 1, web_mouse_callback);
            emscripten_set_mousedown_callback("canvas", 0, 1, web_mouse_callback);
            emscripten_set_mousemove_callback("canvas", 0, 1, web_mouse_callback);
       }

    #endif

    LOG("Inputs: initialized.");

}


//----------------------------------------


void Inputs::ProcessInput(GLFWwindow* window)
{

    //input state

    isDown = numInputs > 0;

    #if DEVELOPMENT == 1 && STANDALONE == 1
        Application::game->physics->enableDebug = G;
    #endif

    //gamepad 

    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

    if (1 == present)
        SetGamepadInputs(GLFW_JOYSTICK_1);

    if (Application::isMobile && cursorReset)
        cursor_callback(Window::s_instance, 0.0f, 0.0f);

    bool isOverlapping = false;

    auto do_check = [&](float x, float y, float width, float height) -> bool {

        bool overlapX = (x + width / 2) >= mouseX && mouseX + width >= x,
             overlapY = (y + height / 2) >= mouseY && mouseY + height >= y;

        return overlapX && overlapY;
    };

    if (Game::GetScene()->virtual_buttons.size())
        for (int i = 0; i < Game::GetScene()->virtual_buttons.size(); i++)
        {

            auto button = Game::GetScene()->virtual_buttons[i];

            if (!button.second->active)
                continue; 

            if (strcmp(button.second->type, "UI") == 0 || strcmp(button.second->type, "sprite") == 0) {
                auto sprite = std::static_pointer_cast<Sprite>(button.second); 
                isOverlapping = do_check(sprite->position.x, sprite->position.y, sprite->texture.FrameWidth, sprite->texture.FrameHeight);  
            }

            if (strcmp(button.second->type, "text") == 0) {
                auto text = std::static_pointer_cast<Text>(button.second);
                isOverlapping = do_check(text->position.x + text->GetTextDimensions().x, text->position.y + text->GetTextDimensions().y, text->GetTextDimensions().x, text->GetTextDimensions().y);
            }

            //set mouse / cursor overlap with object
            
            Game::GetScene()->virtual_buttons[i].first = isOverlapping;

        }
}



//----------------------------------------


void Inputs::cursor_callback(GLFWwindow* window, double xPos, double yPos)
{

    //set cursor object to movement, translate ndc coords to clip space

    auto position = Window::GetNDCToPixel((float)xPos, (float)yPos);
   
    Application::game->inputs->mouseX = position.x;
    Application::game->inputs->mouseY = position.y;

}


//----------------------------------------


void Inputs::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS) {
        Application::game->inputs->SetKeyInputs(true, key, window);  
        Application::game->inputs->numInputs++;
    }

    if (action == GLFW_RELEASE) {
        Application::game->inputs->SetKeyInputs(false, key, window);  
        Application::game->inputs->numInputs--;
    }

}


//----------------------------------------


void Inputs::SetKeyInputs(bool boolean, int key, GLFWwindow* window)
{

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
        case GLFW_KEY_LEFT:
            LEFT = boolean;
        break;
        case GLFW_KEY_RIGHT:
            RIGHT = boolean;
        break;
        case GLFW_KEY_UP:
            UP = boolean;
        break;
        case GLFW_KEY_DOWN:
           DOWN = boolean;
        break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            SHIFT = boolean;
        break;
        case GLFW_KEY_SPACE:
            SPACE = boolean;
        break;
        case GLFW_KEY_ENTER:
            ENTER = boolean;
        break;
        case GLFW_KEY_TAB:
            TAB = boolean;
        break;
        case GLFW_KEY_G:
            G = boolean;
        break;
        case GLFW_KEY_W:
            W = boolean;
        break;
        case GLFW_KEY_A:
            A = boolean;
        break;
        case GLFW_KEY_S:
            S = boolean;
        break;
        case GLFW_KEY_D:
            D = boolean;
        break;
    }
}


//----------------------------------------


void Inputs::SetGamepadInputs(unsigned int joystick)
{

    int axesCount;

    const float* axes = glfwGetJoystickAxes(joystick, &axesCount);

    LEFT = axes[0] > 1;
    RIGHT = axes[1] > 1;

    int buttonCount;

    const unsigned char* buttons = glfwGetJoystickButtons(joystick, &buttonCount);

    if (GLFW_PRESS == buttons[0]) 
        ENTER = true;
    
    else if (GLFW_RELEASE == buttons[0]) 
        ENTER = false;

    if (
        GLFW_PRESS == buttons[1] ||
        GLFW_PRESS == buttons[2]
    )
        SPACE = true;

    else if (
        GLFW_RELEASE == buttons[1] ||
        GLFW_RELEASE == buttons[2]
    )
        SPACE = false;

    if (
        GLFW_PRESS == buttons[3] ||
        GLFW_PRESS == buttons[4]
    )
        SHIFT = true;

    else if (
        GLFW_RELEASE == buttons[3] ||
        GLFW_RELEASE == buttons[4]
    )
        SHIFT = false;

    if (
        GLFW_PRESS == buttons[5] ||
        GLFW_PRESS == buttons[6]
    )
        TAB = true;

    else if (
        GLFW_RELEASE == buttons[5] ||
        GLFW_RELEASE == buttons[6]
    )
        TAB = false;

    if (
        GLFW_PRESS == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        LEFT_CLICK = true;

    else if (
        GLFW_RELEASE == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        LEFT_CLICK = false;

    if (GLFW_PRESS == buttons[10])
        DOWN = true;

    else if (GLFW_RELEASE == buttons[10])
        DOWN = false;

    if (GLFW_PRESS == buttons[11])
        RIGHT = true;

    else if (GLFW_RELEASE == buttons[11])
        RIGHT = false;

    if (GLFW_PRESS == buttons[12])
        UP = true;

    else if (GLFW_RELEASE == buttons[12])
        UP = false;

    if (GLFW_PRESS == buttons[13])
        LEFT = true;

    else if (GLFW_RELEASE == buttons[13])
        LEFT = false;

    for (int i = 0; i < sizeof(buttons); i++) 

        if (buttons[i] == GLFW_PRESS)
            numInputs++;

        else if (buttons[i] == GLFW_RELEASE)
            numInputs--;
}


//----------------------------------------


void Inputs::input_callback(GLFWwindow* window, int input, int action, int mods)
{

    if (input == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) 
        Application::game->inputs->RIGHT_CLICK = true;

    else
        Application::game->inputs->RIGHT_CLICK = false;

    if (input == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
        Application::game->inputs->LEFT_CLICK = true;

    else
        Application::game->inputs->LEFT_CLICK = false;

    if (action == GLFW_PRESS)
        Application::game->inputs->numInputs++;

    if (action == GLFW_RELEASE)
        Application::game->inputs->numInputs--;

}


//-----------------------------------------


void Inputs::ShutDown()
{

    ResetControls();

    if (Application::isMobile) {
        m_initVirtualControls = false;
        Game::GetScene()->virtual_buttons.clear();
    }

    LOG("Inputs: shutdown.");

}


//-----------------------------------------


void Inputs::ResetControls()
{

    cursorReset = true;

    RIGHT_CLICK = false;
    LEFT_CLICK = false;
    LEFT = false;
    RIGHT = false;
    DOWN = false;
    UP = false;
    SHIFT = false;
    TAB = false;
    SPACE = false;
    ENTER = false; 
    W = false;
    A = false;
    S = false;
    D = false;
    G = false;
}





