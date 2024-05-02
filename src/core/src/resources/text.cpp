#include "../../../../build/sdk/include/app.h"


void Text::Init()
{

    if (!gltInit())
    {
        #if DEVELOPMENT == 1
            std::cout << stderr << "Text: Failed to initialize text: " << EXIT_FAILURE << "\n";
        #endif

        return;
	}

    s_buffer = gltCreateText();

    #if DEVELOPMENT == 1
        std::cout << "Text: initialized.\n";
    #endif
}


//--------------------------


void Text::ShutDown() 
{
    
    gltDeleteText(s_buffer);
    gltTerminate();

    #if DEVELOPMENT == 1
        std::cout << "Text: uninitialized.\n";
    #endif
}


//--------------------------


Text::Text(const std::string& content, float x, float y, float scale, glm::vec3 tint)
:
    Entity("text", x, y),
        content(content)
{

    this->scale = glm::vec2(scale);
    this->tint = tint;
    this->m_handle = s_buffer;

    this->SetText(content);

    #if DEVELOPMENT == 1
        std::cout << "Text: " + (std::string)content + " created.\n";
    #endif

}


//--------------------------


Text::~Text() 
{

    #if DEVELOPMENT == 1
        std::cout << "Text: " + (std::string)content + " deleted.\n";
    #endif

}


//--------------------------


void Text::Render()
{

    this->SetText(this->content);

    gltBeginDraw();

    gltColor(this->tint.x, this->tint.y, this->tint.z, this->alpha);

    this->m_model = glm::mat4(1.0f);

    this->m_model = glm::translate(this->m_model, glm::vec3(this->position.x, this->position.y + gltGetTextHeight(this->m_handle, this->scale.y), 0.0f));
    this->m_model = glm::scale(this->m_model, glm::vec3(this->scale.x, this->scale.y, 1.0f));

    glm::highp_mat4 mvp = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight) * this->m_model;

    gltDrawText(this->m_handle, (GLfloat*)&mvp);

    gltEndDraw();


}


//----------------------------


void Text::SetText(const std::string& content) {

    this->content = content;
    gltSetText(this->m_handle, this->content.c_str());
}