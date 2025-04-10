#include <fstream>
#include <sstream>

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
static GLTtext* _GLT_text_buffer;
static std::map<const std::string, GLTtext*> _GLT_text_handles;

//loaded true type fonts via FreeType
static FT_Library* _freetype;



void Text::Init()
{
    //GLtext

    if (!gltInit()) {
        LOG(stderr << "Text: Failed to initialize text: " << EXIT_FAILURE);
	}
    else {
        _GLT_text_buffer = gltCreateText();
        LOG("Text: GLText initialized. (embedded)");
    }

    //FreeType

    _freetype = new FT_Library;

    if (FT_Init_FreeType(_freetype)) {
        LOG("Text: ERROR::FREETYPE: Could not init FreeType Library");
    }
    else {
        LOG("Text: FreeType initialized. (.ttf loading)");
    }
}

//--------------------------


void Text::ShutDown()
{
    _GLT_text_handles.clear();

    gltDeleteText(_GLT_text_buffer);
    gltTerminate();

    if (_freetype) {
        FT_Done_FreeType(*_freetype);
        delete _freetype;
        _freetype = nullptr;
    }

    LOG("Text: uninitialized.");
}


//--------------------------


Text::Text(const std::string& content, 
    float x, 
    float y, 
    const std::string& font, 
    float scale, 
    const Math::Vector3& tint
):
    Entity(TEXT, x, y),
        m_pixel_height(48)
{
    textType = DEFAULT;
    position = { x, y };
    shadowOffset = 0.0f;
    charoffsetX = 0.0f;
    charoffsetY = 0.0f;

    this->scale = { scale, scale };
    this->content = content;
    this->font = font;
    this->tint = tint;

    this->SetText(content);

    //font

    if (font.length())
    {
        if (!_freetype) {
            LOG("Text: cannot create font, FreeType not initialized.");
            return;
        }

        const auto filepath = System::Resources::Manager::GetFilePath(font);

        FT_Face face;

        if (filepath)
        {
            if ((*filepath).empty()) {
                LOG("Text: ERROR::FREETYPE: Failed to load font_name");
                return;
            }

            if (FT_New_Face(*_freetype, (*filepath).c_str(), 0, &face)) {
                LOG("Text: ERROR::FREETYPE: Failed to load font");
                return;
            }

            else {
                LOG("Text: font " + font + " loaded from file. (.ttf)");
            }
        }

        //raw data

        else
        {
            const auto data = System::Resources::Manager::GetResource(font);

            if (data)
            {
                if (FT_New_Memory_Face(*_freetype, data->array_buffer, data->byte_length, 0, &face)) {
                    LOG("Text: ERROR::FREETYPE: Failed to load font");
                    return;
                }
                else {
                    LOG("Text: font " + font + " loaded from memory. (.ttf)");
                }
            }
            else {
                LOG("Text: failed to font load data resource. (" + font + ")");
                return;
            }
        }

        FT_Set_Pixel_Sizes(face, 0, m_pixel_height);

        for (unsigned char c = 0; c < 128; c++)
        {
            //Load character glyph

            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG("Text: ERROR::FREETYTPE: Failed to load Glyph");
                continue;
            }

            unsigned int texture;

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            //disable byte-alignment restriction

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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

            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0,
                0,
                face->glyph->bitmap.width + 10,
                face->glyph->bitmap.rows +10,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            //set texture options

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER /* GL_CLAMP_TO_EDGE */);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER /* GL_CLAMP_TO_EDGE */);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            //align glyphs

            face->glyph->bitmap_top = 29;  

            //now store character for later use

            Character character = {
                texture,
                { (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows },
                { (float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top },
                static_cast<unsigned int>(face->glyph->advance.x)
            };

            m_chars.insert(std::pair<char, Character>(c, character));
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        FT_Done_Face(face);

        textType = FONT;
    }

    //default text

    if (textType == DEFAULT)
        _GLT_text_handles.insert({ ID, _GLT_text_buffer });

    const std::string text_type = textType == DEFAULT ? "default" : font + ".ttf";

    LOG("Text: " + content + " created. (" +  text_type + ")");
}


//--------------------------


Text::~Text()
{
    if (textType == DEFAULT) {
        auto it = std::find_if(_GLT_text_handles.begin(), _GLT_text_handles.end(), [this](const std::pair<std::string, GLTtext*>& text){ return this->ID == text.first; });
        if (it != _GLT_text_handles.end()) {
            it = _GLT_text_handles.erase(std::move(it));
            --it;
        }
    }

    LOG("Text: " + content + " destroyed.");
}


//--------------------------


void* Text::GetGLTPointer()
{
    if (textType == DEFAULT) {
        auto it = _GLT_text_handles.find(ID);
        return it != _GLT_text_handles.end() ? it->second : nullptr;
    }
    return nullptr;
}


//--------------------------


void Text::Render()
{
    const Math::Vector4& pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    glm::mat4 model = glm::mat4(1.0f);

    if (textType == DEFAULT && _GLT_text_buffer)
    {
        auto handle = static_cast<GLTtext*>(GetGLTPointer());

        if (!handle)
            return;

        model = glm::translate(model, { position.x, position.y, 0.0f });
        model = glm::scale(model, { scale.x, scale.y, 1.0f });

        const glm::highp_mat4& mp = glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f) * model;

        SetText(content);
        gltBeginDraw();
        gltColor(tint.x, tint.y, tint.z, alpha);

        gltDrawText(handle, (GLfloat*)&mp);

        #ifndef __EMSCRIPTEN__
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        #endif

        gltEndDraw();
    }

    if (textType == FONT)
    {

        float localX = position.x,
              localY = position.y;

        model = glm::translate(model, { -scale.x, -scale.y, 0.0f });
        model = glm::scale(model, { scale.x, scale.y, 0.0f });

        glm::highp_mat4 proj = glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f) * model;

        const Math::Matrix4 mvp = {
            { proj[0][0], proj[0][1], proj[0][2], proj[0][3] },
            { proj[1][0], proj[1][1], proj[1][2], proj[1][3] },
            { proj[2][0], proj[2][1], proj[2][2], proj[2][3] },
            { proj[3][0], proj[3][1], proj[3][2], proj[3][3] }
        };            
 
        glActiveTexture(GL_TEXTURE0);

        std::stringstream ss(content); 
        std::string line;

        //render each line

        while (getline(ss, line))
        {
            //render each char 

            for (std::string::const_iterator c = line.begin(); c != line.end(); ++c) 
            {            
                //character offsetY spacing

                position.y += charoffsetY;
                
                Character ch = m_chars[*c];

                //update shader

                const auto setShader = [&ch, &mvp, this](const std::string& type = "text") -> void 
                {
                    if (type == "outline") 
                    {
                        SetShader("outline text");
                        shader.SetVec3f("outlineColor", outlineColor.x, outlineColor.y, outlineColor.z);
                        shader.SetFloat("outlineWidth", outlineWidth);     
                        shader.SetFloat("characterWidth", ch.Size.x * scale.x);
                    }
                    else 
                    {
                        SetShader("text");

                        if (type == "shadow") 
                            shader.SetVec3f("textColor", shadowColor.x, shadowColor.y, shadowColor.z);

                        else
                            shader.SetVec3f("textColor", tint.x, tint.y, tint.z);
                    }

                    shader.SetMat4("mvp", mvp);
                    shader.SetFloat("alphaVal", alpha);
                    shader.SetVec2f("scale", scale);
                };

                //update texture

                const auto setTexture = [&, this](float offset = 0.0f) -> void 
                {

                    const float xpos = localX + ch.Bearing.x * scale.x,
                                ypos = localY + position.y - (ch.Size.y - ch.Bearing.y) * scale.y,
                                w = ch.Size.x * scale.x,
                                h = ch.Size.y * scale.y;

                    const float vertices[6][4] = {
                        { xpos + offset,     ypos + h + offset,   0.0f, 1.0f },
                        { xpos + offset,     ypos + offset,       0.0f, 0.0f },
                        { xpos + w + offset, ypos + offset,       1.0f, 0.0f },

                        { xpos + offset,     ypos + h + offset,   0.0f, 1.0f },
                        { xpos + w + offset, ypos + offset,       1.0f, 0.0f },
                        { xpos + w + offset, ypos + h + offset,   1.0f, 1.0f }
                    };

                    glBindVertexArray(m_VAO);
                    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glDrawArrays(GL_TRIANGLES, 0, 6);  
                };

                //shadow pass

                if (shadowEnabled) {
                    setShader("shadow");
                    setTexture(shadowOffset);
                }

                //stroke pass

                if (outlineEnabled) {
                    setShader("outline");
                    setTexture();
                }

                //fill pass

                setShader();
                setTexture();

                localX += (ch.Advance >> 6) * scale.x + charoffsetX;
            }      

            //reset line ledger
            
            localX = position.x;
            localY += (m_pixel_height * 2) * scale.y;   
            line.clear();
        }
    };

    glBindVertexArray(0);

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


//-----------------------------


void Text::SetStroke(bool isOutlined, const Math::Vector3& color, float width)
{
    outlineEnabled = isOutlined;

    if (outlineEnabled) {
        outlineWidth = width;
        outlineColor = color;
    }
}


//-----------------------------


void Text::SetShadow(bool isShadow, const Math::Vector3& color, float offset) {

    shadowEnabled = isShadow;

    if (shadowEnabled) {
        shadowOffset = offset;
        shadowColor = color;
    }
}


//-----------------------------


void Text::SetSlant(float offsetX, float offsetY) {
    charoffsetX = offsetX;
    charoffsetY = offsetY;
}