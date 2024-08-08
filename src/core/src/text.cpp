#include "../../../build/sdk/include/app.h"

//#include <ft2build.h>
//#include FT_FREETYPE_H

//debug text, default embedded
// #define STB_TRUETYPE_IMPLEMENTATION
// #define STB_RECT_PACK_IMPLEMENTATION
// #include "../../../../vendors/stb/stb_rect_pack.h"
// #include "../../../../vendors/stb/stb_truetype.h"


// stbtt_packedchar* comic;

// #include <fstream>


// #define OUTPUT_WIDTH 1920
// #define OUTPUT_HEIGHT 1080
// #define TEXTURE_WIDTH 1024
// #define TEXTURE_HEIGHT 1024
// #define TEXTURE_FONT_SIZE 200.0f

/* Vertex shader ==================================
#version 430
layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec2 texturePos;

uniform vec2 resolution; // Resolution of window/display

uniform vec2 position; // Position of character based on above resolution.
                       // The coordinate is the leftmost pixel at the baseline of character.
                       // Text may go below the baseline (e.g. the bottom of g and j).

uniform vec2 size; // Size of character based on above resolution.

void main() {
    // Adjust inPosition (0,0 => 1,1 square) to the size and position provided by uniform values.
    vec2 pos = (inPosition * size / resolution);
    pos = pos + (position / resolution);

    // Move -1,-1 and scale by 2x since opengl viewport is 2x2 (-1,-1 => 1,1).
    gl_Position = vec4(pos * 2.0 - vec2(1.0, 1.0), 0.0, 1.0);

    // The inPosition with y flipped is used for texture position in fragment shader.
    texturePos = vec2(inPosition.x, (inPosition.y - 1.0) * -1.0);
}
=================================================*/
// const char *vertSource = "#version 330\nlayout(location = 0) in vec2 inPosition; layout(location = 0) out vec2 tex\
// turePos; uniform vec2 resolution; uniform vec2 position; uniform vec2 size; void main() { vec2 pos = (inPosition *\
//  size / resolution); pos = pos + (position / resolution); gl_Position = vec4(pos * 2.0 - vec2(1.0, 1.0), 0.0, 1.0)\
//  ; texturePos = vec2(inPosition.x, (inPosition.y - 1.0) * -1.0); }";

/* Fragment shader ================================
#version 430
layout(location = 0) in vec2 texturePos;
layout(location = 0) out vec4 outColor;

uniform sampler2D fontTexture; // Texture from stb_truetype
uniform vec4 charPosition; // Position and size of character in texture

void main() {
    // Get size of texture in pixels and scale charPosition for texture(...) function
    ivec2 texSize = textureSize(fontTexture, 0);
    vec2 texPos = texturePos * ((charPosition.zw - charPosition.xy) / texSize.x) + charPosition.xy / texSize.y;

    // Get color/alpha mask of fragment from texture.
    float col = texture(fontTexture, texPos).r;

    // Generate a color pattern for text for demo.
    vec3 textColor = vec3(texPos.x, texturePos.y, texPos.y);

    // Set color using mask from front texture as alpha channel.
    outColor = vec4(textColor, col);
}
=================================================*/
// const char* fragSource = "#version 330\nlayout(location = 0) in vec2 texturePos; layout(location = 0) out vec4 out\
// Color; uniform sampler2D fontTexture; uniform vec4 charPosition; void main() { ivec2 texSize = textureSize(fontTex\
// ture, 0); vec2 texPos = texturePos * ((charPosition.zw - charPosition.xy) / texSize.x) + charPosition.xy / texSize\
// .y; float col = texture(fontTexture, texPos).r; vec3 textColor = vec3(texPos.x, texturePos.y, texPos.y); outColor \
// = vec4(textColor, col); }";

// // Load a TTF file into a texture and return the character data.
// stbtt_packedchar* LoadFont(const char* filename) {
//     // Load TTF file into memory.
//     std::ifstream file(filename, std::ios::ate | std::ios::binary); //if (!file.good()) std::cout<<"NOOO"; else  std::cout<<"YESSS";
//     size_t fileSize = (size_t)file.tellg();
//     char* ttfData = (char*)calloc(fileSize + 1, sizeof(char));
//     file.seekg(0);
//     file.read(ttfData, fileSize);
//     file.close();

//     // Pack TTF into pixel data using stb_truetype.
//     stbtt_pack_context packContext;
//     stbtt_packedchar *charData = (stbtt_packedchar*)calloc(126, sizeof(stbtt_packedchar));
//     unsigned char* pixels = (unsigned char*)calloc(TEXTURE_WIDTH * TEXTURE_HEIGHT, sizeof(char));
//     stbtt_PackBegin(&packContext, pixels, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_WIDTH, 1, NULL);

//     // // Pack unicode codepoints 0 to 125 into the texture and character data. If a different starting
//     // // point than 0 is picked then lookups in charData array must be offset by that number.
//     // // With 0-125 the uppercase A will be at charData[65].
//     // // With 32-125 the uppercase A will be at charData[65-32].
//     stbtt_PackFontRange(&packContext, (unsigned char*)ttfData, 0, TEXTURE_FONT_SIZE, 0, 125, charData);
//     stbtt_PackEnd(&packContext);

//     // // Create OpenGL texture with the font pack pixel data.
//     // // Only uses one color channel since font data is a monochrome alpha mask.
//     GLuint fontTexture;
//     glGenTextures(1, &fontTexture);
//     glBindTexture(GL_TEXTURE_2D, fontTexture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//     return charData;
// }

// // Calculate pixel width of a string.
// float TextWidth(const char* text, float height, stbtt_packedchar *charData) {
//     float w = 0.0f;

//     for (int i = 0; i < strlen(text); i++) {
//         stbtt_packedchar c = charData[text[i]];
//         w += c.xadvance;
//     }
//     // Scale width by rendered height to texture generated height ratio for final size.
//     return w * (height / TEXTURE_FONT_SIZE);
// }

// // Render string using glDrawArrays.
// // X indicates the leftmost, center or rightmost point of the based on align (0:left, 1:center, 2:rigght).
// // Y is the baseline of the text.
// void RenderString(const char* text, float x, float y, float height, int align, stbtt_packedchar *charData, GLuint program) {
//     // Scale of actual text compared to size on stb_truetype generated texture.
//     float scale = height / TEXTURE_FONT_SIZE;

//     // Resolution of window/display for shader position calculations.
//     float resolution[2] = {OUTPUT_WIDTH, OUTPUT_HEIGHT};

//     float position[2] = {x, y};
//     if (align == 1) {
//         // Center align
//         position[0] -= TextWidth(text, height, charData) / 2.0f;
//     } else if (align == 2) {
//         // Right align
//         position[0] -= TextWidth(text, height, charData);
//     }

//     glUniform2fv(glGetUniformLocation(program, "resolution"), 1, resolution);
//     for (int i = 0; i < strlen(text); i++) {
//         // Lookup current character data from stb_truetype.
//         stbtt_packedchar c = charData[text[i]];

//         // Position of character in texture.
//         float charPosition[4] = {c.x0, c.y0, c.x1, c.y1};

//         // Find the actual size of character since fonts can be variable width.
//         // 'M' usually is wider than '!', 'L' taller than 'o', etc.
//         // Calculated by substracting start-offset from end-offset.
//         // xoff is start offset from the left.
//         // xoff2 is end offset from the left.
//         // yoff is start offset from the baseline (will be negative if above baseline).
//         // yoff2 is end offset from the baseline.
//         float size[2] = {(c.xoff2 - c.xoff) * scale, (c.yoff2 - c.yoff) * scale};

//         // The actual position of character based on its offset form left/baseline.
//         float renderPos[2] = {position[0] + c.xoff * scale, position[1] - c.yoff2 * scale};

//         glUniform4fv(glGetUniformLocation(program, "charPosition"), 1, charPosition);
//         glUniform2fv(glGetUniformLocation(program, "position"), 1, renderPos);
//         glUniform2fv(glGetUniformLocation(program, "size"), 1, size);

//         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//         // Advance x position to start of next character.
//         position[0] += c.xadvance * scale;
//     }
// }

// int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
//     // Setup basic OpenGL instance and window with GLFW.
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//     glfwWindowHint(GLFW_SAMPLES, 4);
//     GLFWwindow* window = glfwCreateWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "stb_truetype_example", NULL, NULL);
//     glfwMakeContextCurrent(window);
//     glViewport(0, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT);

//     // Setup OpenGL buffer and vertex array with a 2D square that can be rendered
//     // with GL_TRIANGLE_STRIP.
//     GLuint vbo, vao, fbo, fbTexture;
//     float vertexdata[8] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
//     glGenBuffers(1, &vbo);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertexdata, GL_STATIC_DRAW);
//     glGenVertexArrays(1, &vao);
//     glBindVertexArray(vao);

//     // Enable alpha blending since the font will be created as a monochrome mask.
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     // Build shader.
//     GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
//     glShaderSource(vertShader, 1, &vertSource, NULL);
//     glCompileShader(vertShader);
//     GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
//     glShaderSource(fragShader, 1, &fragSource, NULL);
//     glCompileShader(fragShader);
//     GLuint program = glCreateProgram();
//     glAttachShader(program, vertShader);
//     glAttachShader(program, fragShader);
//     glLinkProgram(program);
//     GLint attrib = glGetAttribLocation(program, "inPosition");
//     glEnableVertexAttribArray(attrib);
//     size_t attriboffset = 0;
//     glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(attriboffset));
//     glUseProgram(program);

//     // Load a font and render some text until window is closed.
//     stbtt_packedchar *comic = LoadFont("c:\\windows\\fonts\\candara.ttf");
//     while (true) {
//         double dtime = 0;
//         glfwPollEvents();
//         if (glfwWindowShouldClose(window)) { break; }

//         glClearColor(0.10f, 0.15f, 0.20f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         // Render text.
//         RenderString("Left!", 100, 50, 200, 0, comic, program);
//         RenderString("Center!", OUTPUT_WIDTH / 2.0f, 200, 100, 1, comic, program);
//         RenderString("Right!", OUTPUT_WIDTH -100, 400, 150, 2, comic, program);
//         RenderString("Center", OUTPUT_WIDTH / 2.0f, 600, 300, 1, comic, program);
//         RenderString("Left!", 100, 800, 50, 0, comic, program);
//         RenderString("Right!", OUTPUT_WIDTH - 100, 900, 250, 2, comic, program);

//         glfwSwapBuffers(window);
//     }

//     return 0;
// }
//GLuint program;

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



    // GLuint vbo, vao, fbo, fbTexture;
    // float vertexdata[8] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
    // glGenBuffers(1, &vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertexdata, GL_STATIC_DRAW);
    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);

    // // Enable alpha blending since the font will be created as a monochrome mask.
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build shader.
    //GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(vertShader, 1, &vertSource, NULL);
    //glCompileShader(vertShader);
    //GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    //glShaderSource(fragShader, 1, &fragSource, NULL);
    //glCompileShader(fragShader);
    //program = glCreateProgram();
    //glAttachShader(program, vertShader);
    //glAttachShader(program, fragShader);
    //glLinkProgram(program);
    // GLint attrib = glGetAttribLocation(Shader::Get("text").ID/* program */, "inPosition");
    // glEnableVertexAttribArray(attrib);
    // size_t attriboffset = 0;
    // glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(attriboffset));
    // glUseProgram(Shader::Get("text").ID/* program */);
    // comic = LoadFont("C:\\project_data\\projects\\c++\\spaghyeti_source_engine\\games\\TestGame\\resources\\assets\\fonts\\Silkscreen\\slkscr.ttf"/* "resources\\assets\\fonts\\Silkscreen\\slkscr.ttf" */);

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

   //RenderString("Left!", 100, 50, 200, 0, comic, Shader::Get("text").ID);
}


//----------------------------


void Text::SetText(const std::string& content) {

    this->content = content;
    gltSetText(this->m_handle, this->content.c_str());
}


//-----------------------------


const glm::vec2 Text::GetTextDimensions() 
{

    GLfloat width = gltGetTextWidth(this->m_handle, this->scale.x),
            height = gltGetTextHeight(this->m_handle, this->scale.y);

    return { width, height };
}