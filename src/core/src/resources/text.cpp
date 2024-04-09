#include "../../../../build/include/app.h"


void Text::Init()
{

    if (!gltInit())
    {
        #if DEVELOPMENT == 1
            std::cout << stderr << "Text: Failed to initialize text: " << EXIT_FAILURE << "\n";
        #endif

        return;
	}

    buffer = gltCreateText();

    #if DEVELOPMENT == 1
        std::cout << "Text: initialized.\n";
    #endif
}


//--------------------------


void Text::ShutDown() 
{
    
    gltDeleteText(buffer);
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

    this->m_scale = glm::vec2(scale);
    this->m_tint = tint;
    this->handle = buffer;

    this->SetText(content);

    #if DEVELOPMENT == 1
        std::cout << "Text: text " + (std::string)content + " created.\n";
    #endif

}


//--------------------------


Text::~Text() 
{

    #if DEVELOPMENT == 1
        std::cout << "Text: text deleted.\n";
    #endif

}


//--------------------------


void Text::Render()
{

    this->SetText(this->content);

    gltBeginDraw();

    gltColor(this->m_tint.x, this->m_tint.y, this->m_tint.z, this->m_alpha);

    this->m_model = glm::mat4(1.0f);

    this->m_model = glm::translate(this->m_model, glm::vec3(this->m_position.x, this->m_position.y + gltGetTextHeight(this->handle, this->m_scale.y), 0.0f));
    this->m_model = glm::scale(this->m_model, glm::vec3(this->m_scale.x, this->m_scale.y, 1.0f));

    glm::highp_mat4 mvp = System::Application::game->camera->GetProjectionMatrix(System::Window::m_scaleWidth, System::Window::m_scaleHeight) * this->m_model;

    gltDrawText(this->handle, (GLfloat*)&mvp);

    gltEndDraw();

}


//----------------------------


void Text::SetText(const std::string& content) {

    this->content = content;
    gltSetText(this->handle, this->content.c_str());
}