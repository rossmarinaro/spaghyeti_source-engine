#include <fstream>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"

#include "../../window/renderer.h"
#include "../../../build/sdk/include/vendors/glm/gtc/matrix_transform.hpp" 
#include "../../../build/sdk/include/vendors/glm/gtc/type_ptr.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../vendors/stb/stb_truetype.h"

#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include "../../vendors/glText/gltext.h"


//font data: (data required to render a quad for each glyph)
static std::map<std::string, std::array<stbtt_packedchar, Text::charsToIncludeInFontAtlas>> _packedChars;
static std::map<std::string, std::array<stbtt_aligned_quad, Text::charsToIncludeInFontAtlas>> _alignedQuads;

//default embedded text data
static GLTtext* GLT_text_buffer;
static std::map<std::string, GLTtext*> GLT_text_handles;


void Text::Init() 
{
    if (!gltInit()) {
        LOG(stderr << "Text: Failed to initialize text: " << EXIT_FAILURE);
        return;
	}

    GLT_text_buffer = gltCreateText();

    LOG("Text: initialized.");  
}

//--------------------------


void Text::ShutDown() 
{
    GLT_text_handles.clear();
    _packedChars.clear();
    _alignedQuads.clear();

    gltDeleteText(GLT_text_buffer);
    gltTerminate();

    LOG("Text: uninitialized.");
}


//--------------------------


Text::Text(const std::string& content, float x, float y, const std::string& font, float scale, glm::vec3 tint):
    Entity(TEXT, x, y),
        textType(DEFAULT)
{

    //font

    if (font.length())
    {
        const std::string& filepath = System::Resources::Manager::GetFilePath(font);

        if (filepath != "not found") 
        {
            textType = FONT;
            this->content = content;
            this->font = font;
            this->tint = tint;
            this->scale = glm::vec2(scale);
            position = glm::vec2(x, y);

            glGenBuffers(1, &m_vboID);
            glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
            glBufferData(GL_ARRAY_BUFFER, s_VBO_SIZE, nullptr, GL_DYNAMIC_DRAW);

            glGenVertexArrays(1, &m_vaoID);
            glBindVertexArray(m_vaoID);

            //position attribute:
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
            glEnableVertexAttribArray(0);

            //color attribute:
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            //texCoord attribute:
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void*)(7 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);

            //read font file
            std::ifstream inputStream(filepath.c_str(), std::ios::binary);

            inputStream.seekg(0, std::ios::end);
            auto&& fontFileSize = inputStream.tellg();
            inputStream.seekg(0, std::ios::beg);

            uint8_t* fontDataBuf = new uint8_t[fontFileSize];

            inputStream.read((char*)fontDataBuf, fontFileSize);

            stbtt_fontinfo fontInfo = {};
        
            if (!stbtt_InitFont(&fontInfo, fontDataBuf, 0)) {
                LOG("Font: failed to load: " + filepath);
                return;
            }

            stbtt_packedchar packedChars[charsToIncludeInFontAtlas];
            stbtt_aligned_quad alignedQuads[charsToIncludeInFontAtlas];

            uint8_t* fontAtlasTextureData = new uint8_t[s_fontAtlasWidth * s_fontAtlasHeight];
            
            stbtt_pack_context ctx;

            stbtt_PackBegin(
                &ctx,                                     // stbtt_pack_context (this call will initialize it) 
                (unsigned char*)fontAtlasTextureData,     // Font Atlas texture data
                s_fontAtlasWidth,                           // Width of the font atlas texture
                s_fontAtlasHeight,                          // Height of the font atlas texture
                0,                                        // Stride in bytes
                1,                                        // Padding between the glyphs
                nullptr);

            stbtt_PackFontRange(
                &ctx,                                     // stbtt_pack_context
                fontDataBuf,                              // Font Atlas texture data
                0,                                        // Font Index                                 
                m_fontSize,                                 // Size of font in pixels. (Use STBTT_POINT_SIZE(m_fontSize) to use points) 
                s_codePointOfFirstChar,                     // Code point of the first charecter
                charsToIncludeInFontAtlas,                // No. of charecters to be included in the font atlas 
                packedChars                    // stbtt_packedchar array, this struct will contain the data to render a glyph
            );
            stbtt_PackEnd(&ctx);

            for (int i = 0; i < charsToIncludeInFontAtlas; i++)
            {
                float unusedX, unusedY;

                stbtt_GetPackedQuad(
                    packedChars,              // Array of stbtt_packedchar
                    s_fontAtlasWidth,                      // Width of the font atlas texture
                    s_fontAtlasHeight,                     // Height of the font atlas texture
                    i,                                   // Index of the glyph
                    &unusedX, &unusedY,                  // current position of the glyph in screen pixel coordinates, (not required as we have a different corrdinate system)
                    &alignedQuads[i],         // stbtt_alligned_quad struct. (this struct mainly consists of the texture coordinates)
                    0                                    // Allign X and Y position to a integer (doesn't matter because we are not using 'unusedX' and 'unusedY')
                );
            }
            
            std::array<stbtt_packedchar, charsToIncludeInFontAtlas> pChars;
            std::array<stbtt_aligned_quad, charsToIncludeInFontAtlas> aQuads;  
            std::copy(std::begin(packedChars), std::end(packedChars), std::begin(pChars)); 
            std::copy(std::begin(alignedQuads), std::end(alignedQuads), std::begin(aQuads)); 
            _packedChars.insert({ font, pChars });
            _alignedQuads.insert({ font, aQuads });

            delete[] fontDataBuf;

            if (fontAtlasTextureData) {
            
                glGenTextures(1, &m_fontTextureID);
                glBindTexture(GL_TEXTURE_2D, m_fontTextureID);

                //the given texture data is a single channel 1 byte per pixel data 
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, s_fontAtlasWidth, s_fontAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, fontAtlasTextureData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glBindTexture(GL_TEXTURE_2D, 0);
                
                delete[] fontAtlasTextureData;
            }
        }
    } 

    //default text

    if (textType == DEFAULT) {
        
        this->scale = glm::vec2(scale);
        this->tint = tint;
        this->content = content;
        this->SetText(content);

        GLT_text_handles.insert({ ID, GLT_text_buffer });
    }

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

        auto packed_char_it = std::find_if(_packedChars.begin(), _packedChars.end(), [this](const std::pair<std::string, std::array<stbtt_packedchar, charsToIncludeInFontAtlas>>& ch){ return this->font == ch.first; });
        if (packed_char_it != _packedChars.end()) {
            packed_char_it = _packedChars.erase(std::move(packed_char_it));
            --packed_char_it;
        }

        auto aligned_quads_it = std::find_if(_alignedQuads.begin(), _alignedQuads.end(), [this](const std::pair<std::string, std::array<stbtt_aligned_quad, charsToIncludeInFontAtlas>>& ch){ return this->font == ch.first; });
        if (aligned_quads_it != _alignedQuads.end()) {
            aligned_quads_it = _alignedQuads.erase(std::move(aligned_quads_it));
            --aligned_quads_it;
        }

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


void Text::Render(float projWidth, float projHeight)
{
    if (textType == DEFAULT) 
    {
        glm::mat4 model = glm::mat4(1.0f);
    
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
        model = glm::scale(model, glm::vec3(scale.x, scale.y, 1.0f));

        const glm::highp_mat4 mp = System::Application::game->camera->GetProjectionMatrix(projWidth, projHeight) * model;

        SetText(content);
        gltBeginDraw();
        gltColor(tint.x, tint.y, tint.z, alpha);

        GLTtext* handle = static_cast<GLTtext*>(GetGLTPointer());

        if (handle) 
            gltDrawText(handle, (GLfloat*)&mp);

        #ifndef __EMSCRIPTEN__
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        #endif

        gltEndDraw();
    }

    if (textType == FONT) 
    {

        const float aspectRatio = (projWidth / projHeight); 

        const glm::mat4 projectionMat =  glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f), //System::Application::game->camera->GetProjectionMatrix(projWidth, projHeight),
                        viewProjectionMat = projectionMat; 

        m_vertexIndex = 0;

        constexpr int order[6] = { 0, 1, 2, 0, 2, 3 };
        const float pixelScale = 2.0f / projHeight;

        glm::vec3 localPosition = glm::vec3(System::Window::GetPixelToNDC(position.x, position.y), 0.0f);

        const auto it_packed_chars = _packedChars.find(font);
        const auto it_aligned_quads = _alignedQuads.find(font);

        auto packed_chars = (*it_packed_chars).second;
        auto aligned_quads = (*it_aligned_quads).second;

        for (char ch : content)
        {
            // Check if the charecter glyph is in the font atlas.
            if (ch >= s_codePointOfFirstChar && ch <= s_codePointOfFirstChar + charsToIncludeInFontAtlas)
            {
                if (m_vertices.size() <= m_vertexIndex)
                    m_vertices.resize(m_vertices.size() + 6);

                // Retrive the data that is used to render a glyph of charecter 'ch'
                const stbtt_packedchar* packedChar = &packed_chars[ch - s_codePointOfFirstChar]; 
                const stbtt_aligned_quad* alignedQuad = &aligned_quads[ch - s_codePointOfFirstChar];

                // The units of the fields of the above structs are in pixels, 
                // convert them to a unit of what we want be multilplying to pixelScale  
                const glm::vec2 glyphSize = {
                    (packedChar->x1 - packedChar->x0) * pixelScale * scale.x,
                    (packedChar->y1 - packedChar->y0) * pixelScale * scale.y
                },

                glyphBoundingBoxBottomLeft = {
                    localPosition.x + (packedChar->xoff * pixelScale * scale.x),
                    localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * pixelScale * scale.y
                },

                // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
                glyphVertices[4] = {
                    { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                    { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                    { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y },
                    { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
                },

                glyphTextureCoords[4] = {
                    { alignedQuad->s1, alignedQuad->t0 },
                    { alignedQuad->s0, alignedQuad->t0 },
                    { alignedQuad->s0, alignedQuad->t1 },
                    { alignedQuad->s1, alignedQuad->t1 },
                };

                // Fill the vertex buffer by 6 vertices to render a quad as we are rendering a quad as 2 triangles
                // The order used is in the 'order' array
                // order = [0, 1, 2, 0, 2, 3] is meant to represent 2 triangles: 
                // one by glyphVertices[0], glyphVertices[1], glyphVertices[2] and one by glyphVertices[0], glyphVertices[2], glyphVertices[3]
                for (int i = 0; i < 6; i++) {
                    m_vertices[m_vertexIndex + i].position = glm::vec3(glyphVertices[order[i]], 0.0f);
                    m_vertices[m_vertexIndex + i].color = glm::vec4(tint, alpha);
                    m_vertices[m_vertexIndex + i].texCoord = glyphTextureCoords[order[i]];
                }

                m_vertexIndex += 6;

                // Update the position to render the next glyph specified by packedChar->xadvance.
                localPosition.x += packedChar->xadvance * pixelScale * scale.x;
            }

            // Handle newlines seperately.
            else if(ch == '\n') {
                // advance y by m_fontSize, reset x-coordinate
                localPosition.y -= m_fontSize * pixelScale * scale.y;
                localPosition.x = position.x;
            }
        }
        
        const int shaderID = Shader::Get("text").ID,
                  uniformLoc = glGetUniformLocation(shaderID, "uFontAtlasTexture");

        glBindTexture(GL_TEXTURE_2D, m_fontTextureID);
        glActiveTexture(GL_TEXTURE0);
        glUseProgram(shaderID); 

        const size_t sizeOfVertices = m_vertices.size() * sizeof(Vertex);
        const uint32_t drawCallCount = (sizeOfVertices / s_VBO_SIZE) + 1; //number of chunks
    /*  glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
            glBufferData(GL_ARRAY_BUFFER, s_VBO_SIZE, nullptr, GL_DYNAMIC_DRAW);


            glBindVertexArray(m_vaoID);

            //position attribute:
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
            glEnableVertexAttribArray(0);

            //color attribute:
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            //texCoord attribute:
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void*)(7 * sizeof(float)));
            glEnableVertexAttribArray(2); */
        // Render each chunk of vertex data.
        for (int i = 0; i < drawCallCount; i++)
        {
            const Vertex* data = m_vertices.data() + i * s_VBO_SIZE;
            const uint32_t vertexCount = i == drawCallCount - 1 ? (sizeOfVertices % s_VBO_SIZE) / sizeof(Vertex) : s_VBO_SIZE / (sizeof(Vertex) * 6);

            const int uniformLocation = glGetUniformLocation(shaderID, "uViewProjectionMat");
            glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, glm::value_ptr(viewProjectionMat));

            glBindVertexArray(m_vaoID);
            glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, i == drawCallCount - 1 ? sizeOfVertices % s_VBO_SIZE : s_VBO_SIZE, data);

            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        } 
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


const glm::vec2 Text::GetTextDimensions() 
{
    if (textType == FONT)
        return { 64.0f, m_textHeight };

    const GLTtext* handle = static_cast<GLTtext*>(GetGLTPointer());
    const GLfloat width = gltGetTextWidth(handle, scale.x),
            height = gltGetTextHeight(handle, scale.y);

    return { width, height };
}