#include "../../../../build/sdk/include/app.h"

using namespace System;



void SetMousePosition(float x, float y) 
{
    float ndcX = ((2.0f * x) / Window::s_width - 1.0f),
          ndcY = (1.0f - (2.0f * y) / Window::s_height); 

    Application::game->inputs->mouseX = ndcX * Window::s_scaleWidth + 400; 
    Application::game->inputs->mouseY = -ndcY * Window::s_scaleHeight + 800;
}


//--------------------------------- native input callback to browser

//mouse

#ifdef __EMSCRIPTEN__

    EM_BOOL web_mouse_callback(int eventType, const EmscriptenMouseEvent* event, void* pUserData)
    {

        if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (event->movementX != 0 || event->movementY != 0)) 
        {
            SetMousePosition(event->targetX, event->targetY) 

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

            Application::game->inputs->m_cursorReset = false;

            for (int i = 0; i < event->numTouches; ++i)
            {
                const EmscriptenTouchPoint* touch = &event->touches[i];

                SetMousePosition(touch->targetX, touch->targetY);

                Inputs::cursor_callback(Window::s_instance, touch->targetX, touch->targetY);
                Inputs::input_callback(Window::s_instance, 1, 1, 0);
                Application::game->inputs->numInputs++;
            }
        }

        else
        {
            Application::game->inputs->m_cursorReset = true;
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
    m_cursorX(0), 
    m_cursorY(0),
    m_cursorReset(false), 
    m_initVirtualControls(false)
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

    #if DEVELOPMENT == 1
        Application::game->physics->debug->enable = this->G;
    #endif

    //gamepad

    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

    if (1 == present)
        SetGamepadInputs(GLFW_JOYSTICK_1);


    CheckOverlap();
}


//------------------------------------- cursor object

 
void Inputs::CreateCursor() 
{
    Application::game->cursor = System::Game::CreateGeom(100.0f, 100.0f, 30.0f, 30.0f);
    Application::game->cursor->SetTint(glm::vec3(1.0f, 0.0f, 0.0f)); 
    Application::game->cursor->SetAlpha(0.0f); 
    Application::game->cursor->shader = Shader::GetShader("cursor");
}


//--------------------------------------


void Inputs::RenderCursor()
{

    if (Application::game->cursor != nullptr)
    {

        Application::game->cursor->SetPosition(this->m_cursorX, this->m_cursorY);

        if (Application::isMobile && this->m_cursorReset)
            cursor_callback(Window::s_instance, -100.0f, -100.0f);

        CheckOverlap();

        #if DEVELOPMENT == 1
            Application::game->cursor->SetAlpha(Application::game->physics->debug->enable ? 1.0f : 0.0f);
        #else
            Application::game->cursor->SetAlpha(0.0f);
        #endif

        Application::game->cursor->Render();
    }
}


//------------------------------------- listen for entity overlap


void Inputs::CheckOverlap()
{

    bool isOverlapping = false;

    auto inputs = Application::game->inputs; 

    auto do_check = [&](float x, float y, float width, float height) -> bool {

        float clickareaWidth = 20,
              clickareaHeight = 20;

        bool overlapX = (x + width / 2) >= inputs->mouseX && inputs->mouseX + clickareaWidth >= x,
             overlapY = (y + height / 2) >= inputs->mouseY && inputs->mouseY + clickareaHeight >= y;

        return overlapX && overlapY;
    };

    for (int i = 0; i < Application::game->currentScene->virtual_buttons.size(); i++)
    {

        auto button = Application::game->currentScene->virtual_buttons[i];

        if (!button->active)
            continue; 

        if (strcmp(button->type, "UI") == 0 || strcmp(button->type, "sprite") == 0) {
            auto sprite = std::static_pointer_cast<Sprite>(button);
            isOverlapping = do_check(sprite->position.x, sprite->position.y, sprite->texture.FrameWidth, sprite->texture.FrameHeight);  
        }

        if (strcmp(button->type, "geometry") == 0) {   
            auto geom = std::static_pointer_cast<Geometry>(button);
            isOverlapping = do_check(geom->position.x, geom->position.y, geom->width, geom->height);
        }

        if (strcmp(button->type, "text") == 0) {
            auto text = std::static_pointer_cast<Text>(button);
            isOverlapping = do_check(text->position.x, text->position.y, text->GetTextDimensions()[0], text->GetTextDimensions()[1]);
        }
 
        button->SetTint(isOverlapping ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f));
        button->SetAlpha(isOverlapping ? 0.5f : 1.0f); 

    }
}


//----------------------------------------


void Inputs::ToggleVirtualButtonVisibility(bool visibility) {

    for (const auto &button : Application::game->currentScene->virtual_buttons)
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
            this->LEFT = boolean;
        break;
        case GLFW_KEY_RIGHT:
            this->RIGHT = boolean;
        break;
        case GLFW_KEY_UP:
            this->UP = boolean;
        break;
        case GLFW_KEY_DOWN:
           this->DOWN = boolean;
        break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            this->SHIFT = boolean;
        break;
        case GLFW_KEY_SPACE:
            this->SPACE = boolean;
        break;
        case GLFW_KEY_ENTER:
            this->ENTER = boolean;
        break;
        case GLFW_MOUSE_BUTTON_LEFT:
            this->LEFT_CLICK = boolean;
        break;
        case GLFW_KEY_TAB:
            this->TAB = boolean;
        break;
        case GLFW_KEY_G:
            this->G = boolean;
        break;
    }
}


//----------------------------------------


void Inputs::cursor_callback(GLFWwindow* window, double xPos, double yPos)
{

    if (Application::game->cursor == nullptr)
        return;

    //set cursor object to movement, translate ndc coords to clip space

    SetMousePosition((float)xPos, (float)yPos);

    Application::game->inputs->m_cursorX = (float)xPos;
    Application::game->inputs->m_cursorY = (float)yPos;

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

    this->LEFT = axes[0] > 1;
    this->RIGHT = axes[1] > 1;

    int buttonCount;

    const unsigned char* buttons = glfwGetJoystickButtons(joystick, &buttonCount);

    if (GLFW_PRESS == buttons[0]) 
        this->ENTER = true;
    
    else if (GLFW_RELEASE == buttons[0]) 
        this->ENTER = false;

    if (
        GLFW_PRESS == buttons[1] ||
        GLFW_PRESS == buttons[2]
    )
        this->SPACE = true;

    else if (
        GLFW_RELEASE == buttons[1] ||
        GLFW_RELEASE == buttons[2]
    )
        this->SPACE = false;

    if (
        GLFW_PRESS == buttons[3] ||
        GLFW_PRESS == buttons[4]
    )
        this->SHIFT = true;

    else if (
        GLFW_RELEASE == buttons[3] ||
        GLFW_RELEASE == buttons[4]
    )
        this->SHIFT = false;

    if (
        GLFW_PRESS == buttons[5] ||
        GLFW_PRESS == buttons[6]
    )
        this->TAB = true;

    else if (
        GLFW_RELEASE == buttons[5] ||
        GLFW_RELEASE == buttons[6]
    )
        this->TAB = false;

    if (
        GLFW_PRESS == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        this->LEFT_CLICK = true;

    else if (
        GLFW_RELEASE == buttons[7] ||
        GLFW_PRESS == buttons[8] ||
        GLFW_PRESS == buttons[9]
    )
        this->LEFT_CLICK = false;

    if (GLFW_PRESS == buttons[10])
        this->DOWN = true;

    else if (GLFW_RELEASE == buttons[10])
        this->DOWN = false;

    if (GLFW_PRESS == buttons[11])
        this->RIGHT = true;

    else if (GLFW_RELEASE == buttons[11])
        this->RIGHT = false;

    if (GLFW_PRESS == buttons[12])
        this->UP = true;

    else if (GLFW_RELEASE == buttons[12])
        this->UP = false;

    if (GLFW_PRESS == buttons[13])
        this->LEFT = true;

    else if (GLFW_RELEASE == buttons[13])
        this->LEFT = false;

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
        this->m_initVirtualControls = false;

        for (auto& button : Application::game->currentScene->virtual_buttons)
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

    this->m_cursorReset = true;

    if (!Application::isMobile) {
        this->m_cursorX = -100.0f;
        this->m_cursorY = -100.0f;
    }

    else
        this->LEFT_CLICK = false;

    this->LEFT = false;
    this->RIGHT = false;
    this->DOWN = false;
    this->UP = false;
    this->SHIFT = false;
    this->TAB = false;
    this->SPACE = false;
    this->ENTER = false;
}





