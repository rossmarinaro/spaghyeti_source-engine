#include "../../../../build/include/app.h"

using namespace System;

//--------------------------------- native input callback to browser

//mouse

#ifdef __EMSCRIPTEN__

    EM_BOOL web_mouse_callback(int eventType, const EmscriptenMouseEvent* event, void* pUserData)
    {

        if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (event->movementX != 0 || event->movementY != 0))
            Inputs::cursor_callback(Window::s_instance, event->targetX, event->targetY);

        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            Application::game->inputs->numInputs++;
            Inputs::input_callback(Window::s_instance, 1, 1, 0);
        }

        if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
            Application::game->inputs->numInputs--;
            Inputs::input_callback(Window::s_instance, 0, 0, 0);
        }

        if (eventType == EMSCRIPTEN_EVENT_CLICK)
            Application::game->inputs->m_left_click = true;

        (void)eventType;
        (void)pUserData;

        return EM_FALSE;
    }

//touch


    EM_BOOL web_touch_callback(int eventType, const EmscriptenTouchEvent* event, void* pUserData)
    {

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
        {

            Application::game->inputs->m_left_click = true;

            Application::game->inputs->cursorReset = false;

            for (int i = 0; i < event->numTouches; ++i)
            {
                const EmscriptenTouchPoint* touch = &event->touches[i];

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

Inputs::Inputs()
{

    this->numInputs = 0;

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

    #if DEVELOPMENT == 1
        std::cout << "Inputs: initialized.\n";
    #endif
}


//----------------------------------------


void Inputs::ProcessInput(GLFWwindow* window)
{

    //input state

    this->isDown = this->numInputs > 0 ? true : false;

    //gamepad

    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

    if (1 == present)
        SetGamepadInputs(GLFW_JOYSTICK_1);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {

        glfwSetWindowShouldClose(window, true);

        #if DEVELOPMENT == 1
            std::cout << "Application exit called.\n";
        #endif

        return;
    }

    CheckOverlap();
}


//------------------------------------- cursor object


void Inputs::CreateCursor() 
{
    Application::game->cursor = Application::game->CreateGeom(100.0f, 100.0f, 30.0f, 30.0f);
    Application::game->cursor->SetTint(glm::vec3(1.0f, 0.0f, 0.0f)); 
    Application::game->cursor->SetAlpha(0.0f); 
    Application::game->cursor->m_shader = System::Resources::Manager::shader->GetShader("cursor");
}


//--------------------------------------


void Inputs::RenderCursor()
{

    if (Application::game->cursor != nullptr)
    {

        Application::game->cursor->SetPosition(this->cursorX, this->cursorY);

        if (Application::isMobile && cursorReset)
            cursor_callback(Window::s_instance, -100.0f, -100.0f);

        CheckOverlap();

        #if DEVELOPMENT == 1
            Application::game->cursor->SetAlpha(Application::game->physics->debug->enable ? 1.0f : 0.0f);
        #else
            Application::game->cursor->SetAlpha(0.0f);
        #endif
    }
}


//-------------------------------------


void Inputs::CheckOverlap()
{

    for (int i = 0; i < Application::game->virtual_buttons.size(); i++)
    {

        auto button = Application::game->virtual_buttons[i];

        if (!button->m_active && !button->m_renderable)
            continue;

        bool isOverlapping = Application::game->physics->collisions.CheckCollisions(button, Application::game->cursor, 2);

        button->SetTint(isOverlapping ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f));
        button->SetAlpha(isOverlapping ? 0.5f : 1.0f); 

    }
}


//----------------------------------------


void Inputs::ToggleVirtualButtonVisibility(bool visibility) {

    for (const auto &button : Application::game->virtual_buttons)
        button->SetAlpha(visibility ? 1.0f : 0.0f);
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
            this->m_left = boolean;
        break;
        case GLFW_KEY_RIGHT:
            this->m_right = boolean;
        break;
        case GLFW_KEY_UP:
            this->m_up = boolean;
        break;
        case GLFW_KEY_DOWN:
           this->m_down = boolean;
        break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            this->m_SHIFT = boolean;
        break;
        case GLFW_KEY_SPACE:
            this->m_SPACE = boolean;
        break;
        case GLFW_KEY_ENTER:
            this->m_ENTER = boolean;
        break;
        case GLFW_MOUSE_BUTTON_LEFT:
            this->m_left_click = boolean;
        break;
        case GLFW_KEY_TAB:
            this->m_TAB = boolean;
        break;
        case GLFW_KEY_G:
            this->m_G = boolean;
        break;
    }
}


//----------------------------------------


void Inputs::cursor_callback(GLFWwindow* window, double xPos, double yPos)
{

    if (!Application::isMobile || Application::game->cursor == nullptr)
        return;

    //set cursor object to movement

    Application::game->inputs->cursorX = (float)xPos;
    Application::game->inputs->cursorY = (float)yPos;

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


void Inputs::SetGamepadInputs(unsigned int joystick)
{

    int axesCount;

    const float* axes = glfwGetJoystickAxes(joystick, &axesCount);

    this->m_left = axes[0] > 1;
    this->m_right = axes[1] > 1;

    int buttonCount;

    const unsigned char* buttons = glfwGetJoystickButtons(joystick, &buttonCount);

    if (GLFW_PRESS == buttons[0]) 
        this->m_ENTER = true;
    

    else if (GLFW_RELEASE == buttons[0]) 
        this->m_ENTER = false;

    if (
        GLFW_PRESS == buttons[1] ||
        GLFW_PRESS == buttons[2]
    )
        this->m_SPACE = true;

    else if (
        GLFW_RELEASE == buttons[1] ||
        GLFW_RELEASE == buttons[2]
    )
        this->m_SPACE = false;

    if (
        GLFW_PRESS == buttons[3] ||
        GLFW_PRESS == buttons[4]
    )
        this->m_SHIFT = true;

    else if (
        GLFW_RELEASE == buttons[3] ||
        GLFW_RELEASE == buttons[4]
    )
        this->m_SHIFT = false;

    if (
        GLFW_PRESS == buttons[5] ||
        GLFW_PRESS == buttons[6]
    )
        this->m_TAB = true;

    else if (
        GLFW_RELEASE == buttons[5] ||
        GLFW_RELEASE == buttons[6]
    )
        this->m_TAB = false;

    if (
        GLFW_PRESS == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        this->m_left_click = true;

    else if (
        GLFW_RELEASE == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        this->m_left_click = false;

    if (GLFW_PRESS == buttons[10])
        this->m_down = true;

    else if (GLFW_RELEASE == buttons[10])
        this->m_down = false;

    if (GLFW_PRESS == buttons[11])
        this->m_right = true;

    else if (GLFW_RELEASE == buttons[11])
        this->m_right = false;

    if (GLFW_PRESS == buttons[12])
        this->m_up = true;

    else if (GLFW_RELEASE == buttons[12])
        this->m_up = false;

    if (GLFW_PRESS == buttons[13])
        this->m_left = true;

    else if (GLFW_RELEASE == buttons[13])
        this->m_left = false;

    for (int i = 0; i < sizeof(buttons); i++) 

        if (buttons[i] == GLFW_PRESS)
            this->numInputs++;

        else if (buttons[i] == GLFW_RELEASE)
            this->numInputs--;
}


//----------------------------------------


void Inputs::input_callback(GLFWwindow* window, int input, int action, int mods)
{

    if (input == GLFW_MOUSE_BUTTON_LEFT || input == 1)
    {
        if (action == GLFW_PRESS || action == 1)
            return;
        else
            Application::game->inputs->ResetControls();
    }

    else
       Application::game->inputs->ResetControls();

    if (action == GLFW_PRESS)
        Application::game->inputs->numInputs++;

    if (action == GLFW_RELEASE)
        Application::game->inputs->numInputs--;

}



//-----------------------------------------



void Inputs::ShutDown()
{

    ResetControls();

    if (Application::isMobile)
    {
        this->initVirtualControls = false;

        for (auto &button : Application::game->virtual_buttons)
            if (button) {
                button.reset();
                button = nullptr;
            }
    }

    #if DEVELOPMENT == 1
        std::cout << "Inputs: shutdown.\n";
    #endif
}


//-----------------------------------------


void Inputs::ResetControls()
{

    this->cursorReset = true;

    if (!Application::isMobile) {
        this->cursorX = -100.0f;
        this->cursorY = -100.0f;
    }

    else
        this->m_left_click = false;

    this->m_left = false;
    this->m_right = false;
    this->m_down = false;
    this->m_up = false;
    this->m_SHIFT = false;
    this->m_TAB = false;
    this->m_SPACE = false;
    this->m_ENTER = false;
}





