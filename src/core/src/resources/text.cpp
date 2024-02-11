#include "../app/app.h"


void Text::Init()
{

    if (!gltInit()) {
        std::cout << stderr << "Text: Failed to initialize text: " << EXIT_FAILURE << "\n";

        return;
	}

    std::cout << "Text: initialized.\n";
}


//--------------------------


void Text::ShutDown() {

    gltTerminate();

    std::cout << "Text: uninitialized.\n";
}


//-------------------------- 


Text::Text(std::string content, float x, float y, float scale, glm::vec3 tint)
:
    Entity("text", glm::vec2(x, y)),
        content(content)
{

    this->m_scale = glm::vec2(scale);
    this->m_tint = tint;
    this->buffer = gltCreateText();

    this->SetText(content);

    std::cout << "Text: text " + (std::string)content + " created.\n";

}


//--------------------------


Text::~Text() {

    gltDeleteText(this->buffer);

    std::cout << "Text: text deleted.\n";

}


//--------------------------


void Text::Render()
{

    this->SetText(this->content);

    gltBeginDraw();

    gltColor(this->m_tint.x, this->m_tint.y, this->m_tint.z, this->m_alpha);

    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(this->m_position.x, this->m_position.y + gltGetTextHeight(this->buffer, this->m_scale.y), 0.0f));
    model = glm::scale(model, glm::vec3(this->m_scale.x, this->m_scale.y, 1.0f));
    
    glm::highp_mat4 mvp = System::Application::game->camera->GetProjectionMatrix(System::Window::m_scaleWidth, System::Window::m_scaleHeight) * model;
    
    gltDrawText(this->buffer, (GLfloat*)&mvp); 

    gltEndDraw();

}


//----------------------------


void Text::SetText(const std::string &content) {

    this->content = content;
    gltSetText(this->buffer, this->content.c_str()); 
}


