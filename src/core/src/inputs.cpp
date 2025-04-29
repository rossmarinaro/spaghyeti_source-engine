#include "../../../build/sdk/include/app.h"
#include "../../shared/renderer.h"

#ifdef __EMSCRIPTEN__
    #include "../../../build/sdk/include/window.h"

#endif

using namespace System;

 
//--------------------------------- native input callback to browser

//mouse

#ifdef __EMSCRIPTEN__

    EM_BOOL web_mouse_callback(int eventType, const EmscriptenMouseEvent* event, void* pUserData)
    {
        if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (event->movementX != 0 || event->movementY != 0)) 
            Renderer::cursor_callback(Renderer::GLFW_window_instance, event->targetX, event->targetY);
        
        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            Application::game->inputs->numInputs++; 
            Renderer::input_callback(Renderer::GLFW_window_instance, 1, 1, 0);
        }

        if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
            Application::game->inputs->numInputs--;
            Renderer::input_callback(Renderer::GLFW_window_instance, 0, 0, 0);
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
        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART || EMSCRIPTEN_EVENT_TOUCHMOVE)
        {
            Application::game->inputs->LEFT_CLICK = true;
            Application::game->inputs->cursorReset = false; 

            for (int i = 0; i < event->numTouches; ++i) {
                const EmscriptenTouchPoint* touch = &event->touches[i];

                Renderer::cursor_callback(Renderer::GLFW_window_instance, touch->targetX, touch->targetY); //canvasX, clientX
                Renderer::input_callback(Renderer::GLFW_window_instance, 1, 1, 0);

                Application::game->inputs->numInputs++;
            }
        }

        else {
            Application::game->inputs->cursorReset = true;
            Renderer::input_callback(Renderer::GLFW_window_instance, 0, 0, 0);
            Renderer::cursor_callback(Renderer::GLFW_window_instance, 0, 0); 
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

        emscripten_set_touchstart_callback("canvas", 0, 1, web_touch_callback);
        emscripten_set_touchend_callback("canvas", 0, 1, web_touch_callback);
        emscripten_set_touchmove_callback("canvas", 0, 1, web_touch_callback);

        emscripten_set_click_callback("canvas", 0, 1, web_mouse_callback);
        emscripten_set_mouseup_callback("canvas", 0, 1, web_mouse_callback);
        emscripten_set_mousedown_callback("canvas", 0, 1, web_mouse_callback);
        emscripten_set_mousemove_callback("canvas", 0, 1, web_mouse_callback);

    #endif

    LOG("Inputs: initialized.");

}


//----------------------------------------


void Inputs::ProcessInput()
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

    auto do_check = [&](float x, float y, float width, float height) -> bool {

        bool overlapX = (x + width / 2) >= mouseX && mouseX + width >= x,
             overlapY = (y + height / 2) >= mouseY && mouseY + height >= y;

        return overlapX && overlapY;
    };

    //set mouse / cursor overlap with object

    auto& virtualButtons = Game::GetScene()->virtual_buttons;

    if (virtualButtons.size())
        for (int i = 0; i < virtualButtons.size(); i++)
        {  
            for (auto& UI : Game::GetScene()->UI) 
            {
                if (UI->ID != virtualButtons[i].second) 
                    continue;

                if (UI->GetType() == Entity::UI || UI->GetType() == Entity::SPRITE) {
                    const auto sprite = std::static_pointer_cast<Sprite>(UI); 
                    virtualButtons[i].first = do_check(sprite->position.x * sprite->scale.x, sprite->position.y * sprite->scale.y, sprite->texture.FrameWidth * sprite->scale.x, sprite->texture.FrameHeight * sprite->scale.y);
                }

                if (UI->GetType() == Entity::TEXT) {
                    const auto text = std::static_pointer_cast<Text>(UI);
                    virtualButtons[i].first = do_check((text->position.x + text->GetTextDimensions().x), (text->position.y + text->GetTextDimensions().y), text->GetTextDimensions().x, text->GetTextDimensions().y);
                }   
            } 
        } 
}


//----------------------------------------


void Inputs::SetKeyInputs(bool boolean, int key, void* window_ptr)
{            
    switch (key)
    {
        case GLFW_KEY_ESCAPE: {
            GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr);
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
        case GLFW_KEY_LEFT: LEFT = boolean; break;
        case GLFW_KEY_RIGHT: RIGHT = boolean; break;
        case GLFW_KEY_UP: UP = boolean; break;
        case GLFW_KEY_DOWN: DOWN = boolean; break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT: SHIFT = boolean; break;
        case GLFW_KEY_SPACE: SPACE = boolean; break;
        case GLFW_KEY_ENTER: ENTER = boolean; break;
        case GLFW_KEY_TAB: TAB = boolean; break;
        case GLFW_KEY_G: G = boolean; break;
        case GLFW_KEY_W: W = boolean; break;
        case GLFW_KEY_A: A = boolean; break;
        case GLFW_KEY_S: S = boolean; break;
        case GLFW_KEY_D: D = boolean; break;
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


//-----------------------------------------


void Inputs::ShutDown()
{

    ResetControls();

    if (Application::events->isMobile) {
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





