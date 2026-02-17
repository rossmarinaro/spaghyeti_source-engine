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
#include FT_STROKER_H

//default embedded gltext data
static GLTtext* _GLT_text_buffer;
static std::map<const std::string, GLTtext*> _GLT_text_handles;

//loaded truetype fonts via FreeType
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
    shadowOffsetX = 0.0f;
    shadowOffsetY = 0.0f;
    charoffsetX = 0.0f;
    charoffsetY = 0.0f;
    shadowEnabled = false;

    this->scale = { scale, scale };
    this->content = content;
    this->font = font;
    this->tint = tint;

    SetText(content);

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

            if (FT_Load_Char(face, c, /* FT_LOAD_DEFAULT */ FT_LOAD_RENDER)) {
                LOG("Text: ERROR::FREETYTPE: Failed to load Glyph");
                continue;
            }

            /* FT_Stroker stroker;
            FT_Stroker_New(*_freetype, &stroker);
            FT_Stroker_Set(stroker, 2 * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
            FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
            FT_Glyph glyph;
            FT_Get_Glyph(face->glyph, &glyph);
            FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
            FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph); */

            unsigned int texture;

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            //disable byte-alignment restriction

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            
            #ifndef __EMSCRIPTEN__
                glGenerateMipmap(GL_TEXTURE_2D);
            #endif

            unsigned int internal_format = GL_RED;
       
            #ifdef __EMSCRIPTEN__
                internal_format = GL_R8;
            #endif

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, /* GL_CLAMP_TO_BORDER */ GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, /* GL_CLAMP_TO_BORDER */ GL_CLAMP_TO_EDGE);
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

        SetShader("text");
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

    if (textType == FONT) 
    {
        for (const auto& character : m_chars) 
            glDeleteTextures(1, &character.second.TextureID);    

        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
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
    const Math::Matrix4& vm = System::Application::game->camera->GetViewMatrix((System::Application::game->camera->GetPosition()->x * scrollFactor.x * scale.x), (System::Application::game->camera->GetPosition()->y * scrollFactor.y * scale.y));

    glm::mat4 model(1.0f);

    const auto get_mvp = [&vm, &pm, this] (glm::mat4& model) -> const Math::Matrix4 
    {
        const glm::mat4 view = m_isStatic ? glm::mat4(1.0f) : glm::mat4({ vm.a.r, vm.a.g, vm.a.b, vm.a.a }, { vm.b.r, vm.b.g, vm.b.b, vm.b.a }, { vm.c.r, vm.c.g, vm.c.b, vm.c.a }, { vm.d.r, vm.d.g, vm.d.b, vm.d.a }), 
                        proj = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f),
                        mvp = proj * view * model;

        return { 
            { mvp[0][0], mvp[0][1], mvp[0][2], mvp[0][3] }, 
            { mvp[1][0], mvp[1][1], mvp[1][2], mvp[1][3] },   
            { mvp[2][0], mvp[2][1], mvp[2][2], mvp[2][3] },  
            { mvp[3][0], mvp[3][1], mvp[3][2], mvp[3][3] }
        };
    };

    //render default gltext

    if (textType == DEFAULT && _GLT_text_buffer)
    {
        auto handle = static_cast<GLTtext*>(GetGLTPointer());

        if (!handle)
            return;

        SetText(content);
    
        //shadow pass

        if (shadowEnabled) 
        {
            glm::mat4 _model(1.0f);
            _model = glm::translate(_model, { position.x + shadowOffsetX, position.y + shadowOffsetY, 0.0f });
            _model = glm::scale(_model, { scale.x, scale.y, 1.0f });

            const Math::Matrix4 mvp = get_mvp(_model);

            gltBeginDraw();
            gltColor(shadowColor.x, shadowColor.y, shadowColor.z, alpha);
            gltOutlineColor(shadowColor.x, shadowColor.y, shadowColor.z, outlineEnabled ? alpha : 0.0f); 
            gltDrawText(handle, (GLfloat*)&mvp);
            gltEndDraw();
        }

        //fill pass

        model = glm::translate(model, { position.x, position.y, 0.0f });
        model = glm::scale(model, { scale.x, scale.y, 1.0f }); 

        const Math::Matrix4 mvp = get_mvp(model);

        gltBeginDraw();
        gltColor(tint.x, tint.y, tint.z, alpha);
        gltOutlineColor(outlineColor.x, outlineColor.y, outlineColor.z, outlineEnabled ? alpha : 0.0f);
        gltDrawText(handle, (GLfloat*)&mvp);

        gltEndDraw();
    }

    //render freetype font

    if (textType == FONT)
    {
        float localX = position.x,
              localY = position.y;

        model = glm::translate(model, { -scale.x, -scale.y, 0.0f });
        model = glm::scale(model, { scale.x, scale.y, 0.0f });

        glm::highp_mat4 proj = glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f) * model;

        const Math::Matrix4 mvp = {
            { proj[0][0], proj[0][1], proj[0][2], proj[0][3] },
            { proj[1][0], proj[1][1], proj[1][2], proj[1][3] },
            { proj[2][0], proj[2][1], proj[2][2], proj[2][3] },
            { proj[3][0], proj[3][1], proj[3][2], proj[3][3] }
        };            
 
        glActiveTexture(GL_TEXTURE0);

        std::stringstream ss(content); 
        std::string line;

        //render each line in current text content

        while (getline(ss, line))
        {
            //render each char 

            for (std::string::const_iterator c = line.begin(); c != line.end(); ++c) 
            {            
                //character offsetY spacing

                position.y += charoffsetY;
                
                Character ch = m_chars[*c];

                //update shader

                const auto setShader = [&ch, &mvp, this](const std::string& type = "") -> void 
                {
                    auto shader = Graphics::Shader::Get("text");

                    if (type == "outline") 
                        shader.SetVec3f("textColor", outlineColor.x, outlineColor.y, outlineColor.z);
            
                    else {
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

                const auto setTexture = [&, this](float offsetX = 0.0f, float offsetY = 0.0f) -> void 
                {
                    const float xpos = localX + ch.Bearing.x * scale.x,
                                ypos = localY + position.y - (ch.Size.y - ch.Bearing.y) * scale.y,
                                w = ch.Size.x * scale.x,
                                h = ch.Size.y * scale.y;

                    //todo: vertical text support

                    /* const float xpos = position.x + ch.Bearing.x * scale.x,
                                ypos = position.y + (m_chars['H'].Bearing.y - ch.Bearing.y) * scale.y,
                                w = ch.Size.x * scale.x,
                                h = ch.Size.y * scale.y; */

                    const float vertices[6][4] = {
                        { xpos + offsetX,     ypos + h + offsetY,   0.0f, 1.0f },
                        { xpos + offsetX,     ypos + offsetY,       0.0f, 0.0f },
                        { xpos + w + offsetX, ypos + offsetY,       1.0f, 0.0f },

                        { xpos + offsetX,     ypos + h + offsetY,   0.0f, 1.0f },
                        { xpos + w + offsetX, ypos + offsetY,       1.0f, 0.0f },
                        { xpos + w + offsetX, ypos + h + offsetY,   1.0f, 1.0f }
                    };

                    glBindVertexArray(m_VAO);
                    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glDrawArrays(GL_TRIANGLES, 0, 6);  



    //    auto renderer = System::Application::renderer;
                 
    // Math::Graphics::Vertex vertices[4];
    
    // vertices[0].x = xpos + offsetX;
    // vertices[0].y = ypos + h + offsetY;
    // vertices[0].u = 0.0f;
    // vertices[0].v = 1.0f;

    // vertices[1].x = xpos + offsetX;
    // vertices[1].y = ypos + offsetY;
    // vertices[1].u = 0.0f;
    // vertices[1].v = 0.0f;

    // vertices[2].x = xpos + w + offsetX;
    // vertices[2].y = ypos + offsetY;
    // vertices[2].u = 1.0f;
    // vertices[2].v = 0.0f;

    // vertices[3].x = xpos + offsetX;
    // vertices[3].y = ypos + h + offsetY;
    // vertices[3].u = 0.0f;
    // vertices[3].v = 1.0f;

   

    // for (int i = 0; i < 4; i++) 
    // {       
    //     vertices[i].texID = 0.0f;
    //     vertices[i].z = depth;
    //     vertices[i].rotation = rotation;
    //     vertices[i].scaleX = scale.x;
    //     vertices[i].scaleY = scale.y;
    //     vertices[i].r = 1.0f;
    //     vertices[i].g = 1.0f;
    //     vertices[i].b = 1.0f;
    //     vertices[i].a = 1.0f;
        
    //     std::memcpy(vertices[i].modelView, glm::value_ptr(model), sizeof(vertices[i].modelView));
    // }  

    //add the vertices to the renderer's vector and increase index count by 6
    
    //std::copy(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]), std::back_inserter(renderer->vertices));

    //renderer->indexCount += 6;
                };

                //shadow pass

                if (shadowEnabled) {
                    setShader("shadow");
                    setTexture(shadowOffsetX, shadowOffsetY);
                }

                //stroke pass

                if (outlineEnabled) {
                    setShader("outline");
                    setTexture(0.0f, -outlineWidth);        
                    setTexture(0.0f, outlineWidth);
                    setTexture(-outlineWidth, 0.0f);
                    setTexture(outlineWidth, 0.0f);
                }

                //fill pass

                setShader();
                setTexture();

                localX += (ch.Advance >> 6) * scale.x + charoffsetX;
            }      

            //reset ledger, moving down a line 
            
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
    if (textType == DEFAULT) 
    {
        const GLTtext* handle = static_cast<GLTtext*>(GetGLTPointer());
        const GLfloat width = gltGetTextWidth(handle, scale.x),
                      height = gltGetTextHeight(handle, scale.y);

        return { width, height };
    }
    
    else if (textType == FONT) 
    {
        float w = 0.0f, 
              h = 0.0f;

        for (const auto& ch : m_chars) {
            w += ch.second.Size.x * scale.x;
            h += ch.second.Size.y * scale.y;
        }

        return { w, h };
    }

    return { 0.0f, 0.0f };
}


//-----------------------------


void Text::SetStroke(bool isOutlined, const Math::Vector3& color, float width)
{
    outlineEnabled = isOutlined;

    if (outlineEnabled) {
        outlineWidth = width;
        outlineColor = color;
    }

    if (outlineWidth > 6)
        outlineWidth = 6;
}


//-----------------------------


void Text::SetShadow(bool isShadow, const Math::Vector3& color, float offsetX, float offsetY) {

    shadowEnabled = isShadow;

    if (shadowEnabled) {
        shadowOffsetX = offsetX;
        shadowOffsetY = offsetY;
        shadowColor = color;
    }
}


//-----------------------------


void Text::SetSlant(float offsetX, float offsetY) {
    charoffsetX = offsetX;
    charoffsetY = offsetY;
}