#include <fstream>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"

#include "../../shared/renderer.h"
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/matrix_transform.hpp"
#include "../../vendors/glm/gtc/type_ptr.hpp"

#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include "../../vendors/glText/gltext.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H


//default embedded text data
static GLTtext* GLT_text_buffer;
static std::map<const std::string, GLTtext*> GLT_text_handles;

    

void Text::Init() 
{
    if (!gltInit()) {
        LOG(stderr << "Text: Failed to initialize text: " << EXIT_FAILURE);
        return;
	}

    GLT_text_buffer = gltCreateText();

    LOG("Text: GLText initialized. (embedded)");  
}

//--------------------------


void Text::ShutDown() 
{
    GLT_text_handles.clear();

    gltDeleteText(GLT_text_buffer);
    gltTerminate();

    LOG("Text: uninitialized.");
}


//--------------------------


Text::Text(const std::string& content, float x, float y, const std::string& font, float scale, const Math::Vector3& tint):
    Entity(TEXT, x, y),
        textType(DEFAULT)
{

    position = { x, y };
    point = scale;
    this->scale = { scale, scale };
    this->content = content;
    this->font = font;
    this->tint = tint;

    this->SetText(content);

    //font

    if (font.length())
    {
        const std::string& filepath = System::Resources::Manager::GetFilePath(font);
        shader = Graphics::Shader::Get("text"); 

        if (filepath != "not found") 
        {
            textType = FONT;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // FreeType
            // --------
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            
            if (FT_Init_FreeType(&ft)) {
            LOG("ERROR::FREETYPE: Could not init FreeType Library");
                return;
            }

            // find path to font
    
            if (ffilepath.empty()) {
                LOG("ERROR::FREETYPE: Failed to load font_name");
                return;
            }
            
            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
                LOG("ERROR::FREETYPE: Failed to load font");
                return;
            }

            else 
            {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);

                // disable byte-alignment restriction
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (unsigned char c = 0; c < 128; c++)
                {
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                        LOG("ERROR::FREETYTPE: Failed to load Glyph");
                        continue;
                    }
                    // generate texture
                    unsigned int texture;
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RED,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        0,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer
                    );
                    // set texture options
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    // now store character for later use
                    Character character = {
                        texture,
                        { face->glyph->bitmap.width, face->glyph->bitmap.rows },
                        { face->glyph->bitmap_left, face->glyph->bitmap_top },
                        static_cast<unsigned int>(face->glyph->advance.x)
                    };
                    Characters.insert(std::pair<char, Character>(c, character));
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            // destroy FreeType once we're finished
            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            // configure VAO/VBO for texture quads
            // -----------------------------------
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        } 
    }
    //default text

    if (textType == DEFAULT) 
        GLT_text_handles.insert({ ID, GLT_text_buffer });
    
    const std::string text_type = textType == DEFAULT ? "default" : font + ".ttf";

    LOG("Text: " + content + " created. (" +  text_type + ")");
}


//--------------------------


Text::~Text() 
{
    if (textType == DEFAULT) {
        auto it = std::find_if(GLT_text_handles.begin(), GLT_text_handles.end(), [this](const std::pair<std::string, GLTtext*>& text){ return this->ID == text.first; });
        if (it != GLT_text_handles.end()) {
            it = GLT_text_handles.erase(std::move(it));
            --it;
        }
    }

    if (textType == FONT) 
    {
    }

    LOG("Text: " + content + " destroyed.");
}


//--------------------------


void* Text::GetGLTPointer() 
{
    if (textType == DEFAULT) {
        auto it = GLT_text_handles.find(ID);
        return it != GLT_text_handles.end() ? it->second : nullptr;
    }
    return nullptr;
}


//--------------------------


void Text::Render()
{
    const Math::Vector4& pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    glm::mat4 model = glm::mat4(1.0f);

    if (textType == DEFAULT && GLT_text_buffer) 
    {   
        model = glm::translate(model, { position.x, position.y, 0.0f });
        model = glm::scale(model, { scale.x, scale.y, 1.0f });
        
        const glm::highp_mat4& mp = glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f) * model;

        SetText(content);
        gltBeginDraw();
        gltColor(tint.x, tint.y, tint.z, alpha);

        auto handle = static_cast<GLTtext*>(GetGLTPointer());

        if (handle) 
            gltDrawText(handle, (GLfloat*)&mp);

        #ifndef __EMSCRIPTEN__
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        #endif

        gltEndDraw();
    }

    if (textType == FONT) 
    {                    
        model = glm::scale(model, { scale.x, scale.y, 1.0f });

        const glm::highp_mat4& proj = glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f) * model;

        shader.SetMat4("mvp", {
            { proj[0][0], proj[0][1], proj[0][2], proj[0][3] }, 
            { proj[1][0], proj[1][1], proj[1][2], proj[1][3] },   
            { proj[2][0], proj[2][1], proj[2][2], proj[2][3] },  
            { proj[3][0], proj[3][1], proj[3][2], proj[3][3] }
        });

        shader.SetVec3f("textColor", tint.x, tint.y, tint.z);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        float x = position.x;

        //iterate through all characters

        for (std::string::const_iterator c = content.begin(); c != content.end(); c++) 
        {
            Character ch = Characters[*c];

            const float xpos = x + ch.Bearing.x * scale.x,
                        ypos = position.y - (ch.Size.y - ch.Bearing.y) * scale.y,
                        w = ch.Size.x * scale.x,
                        h = ch.Size.y * scale.y;

            //update VBO for each character

            const float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },            
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },

                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }           
            };

            //render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            //render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            //now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale.x;// bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


//----------------------------


void Text::SetText(const std::string& content) {
    this->content = content;
    if (textType == DEFAULT) {
        GLTtext* handle = static_cast<GLTtext*>(GetGLTPointer());
        gltSetText(handle, this->content.c_str());
    } 
}


//-----------------------------


const Math::Vector2 Text::GetTextDimensions() 
{
    const GLTtext* handle = static_cast<GLTtext*>(GetGLTPointer());
    const GLfloat width = gltGetTextWidth(handle, scale.x),
                  height = gltGetTextHeight(handle, scale.y);

    return { width, height };
}