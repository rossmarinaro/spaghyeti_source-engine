#include "../../../../build/sdk/include/app.h"
#include "./gui.h"
#include "../editor.h"

#include "../assets/embedded/images/data_src.h"
#include "../assets/embedded/images/audio_src.h"
#include "../assets/embedded/images/editor_logo.h"
#include "../assets/embedded/images/icon_large.h"
#include "../assets/embedded/images/folder_src.h"

using namespace /* SPAGHYETI_CORE */ System;
using namespace editor;

//------------------------------------- launch GUI

void GUI::Launch()
{

    //Poll and handle events (inputs, window resize, etc.)
    //You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    //- When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    //- When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    //Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

    Editor::Log("IMGui Version: " + std::to_string((int)IMGUI_CHECKVERSION()));

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !

    io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("src/assets/fonts/Silkscreen/slkscr.ttf", 10.0f);

    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    ImGui::StyleColorsDark();

    //setup platform/renderer backends

    ImGui_ImplGlfw_InitForOpenGL(Window::s_instance, true);
    ImGui_ImplOpenGL3_Init(Window::s_glsl_version);

    static const char* checker_vertex = 

        "#version 330 core\n"

        "layout (location = 0) in vec2 vert;\n"

        "out vec2 position;\n"

        "void main()\n"
        "{\n"    
            "gl_Position = vec4(vert.xy, 0.0, 1.0);\n" 
        "}\n"; 


    static const char* checker_fragment =  

        "#version 330 core\n"

        "precision mediump float;\n"

        "uniform float alphaVal;\n"
        "uniform float pitch;\n"

        "void main()\n" 
        "{\n"    

            "vec2 pitch = vec2(pitch, pitch);\n"

            "if (mod(gl_FragCoord.x, pitch[0]) < 1. ||\n"
                "mod(gl_FragCoord.y, pitch[1]) < 1.) {\n"
                "gl_FragColor = vec4(0.25, 0.25, 0.25, alphaVal);\n"
            "} else {\n"
                "gl_FragColor = vec4(0.);\n"
                
            "}\n"
        "}\n";


    Shader::Load("grid", checker_vertex, checker_fragment); 

    s_grid = std::make_unique<Geometry>(-10, -10, 1500, 1500);
    s_grid->shader = Shader::Get("grid");

    //load embedded assets

    System::Resources::Manager::LoadRawImage("editor logo", Assets::Images::editor_logo, 66, 65, 4);
    System::Resources::Manager::LoadRawImage("icon large", Assets::Images::icon_large, 211, 126, 4);
    System::Resources::Manager::LoadRawImage("audio src", Assets::Images::audio_src, 75, 70, 3);
    System::Resources::Manager::LoadRawImage("data src", Assets::Images::data_src, 75, 70, 4);
    System::Resources::Manager::LoadRawImage("folder src", Assets::Images::folder_src, 202, 202, 4);

    System::Resources::Manager::RegisterTextures();

    glfwSetScrollCallback(System::Window::s_instance, scroll_callback);

    s_cursor = std::make_unique<Geometry>(100.0f, 100.0f, 30.0f, 30.0f);
    s_cursor->SetTint(glm::vec3(1.0f, 0.0f, 0.0f));  
    s_cursor->SetAlpha(0.0f);

    Editor::Log("GUI launched.");

}


//----------------------------------- alignment util


void GUI::AlignForWidth(float width, float alignment)
{

    ImGuiStyle& style = ImGui::GetStyle();

    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;

    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

}


//----------------------------------- render GUI


void GUI::Render()
{

    if (!s_running)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    if (s_show_init)
        ShowOptionsInit();

    else
        RenderDockSpace();

    if (s_show_quit)
        ShowOptionsQuit();

    if (Editor::events.buildFlag)
        ShowOptionsSave(false);

    else if (Editor::events.saveFlag)
        ShowOptionsSave(true);

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (s_grid)
       s_grid->shader.SetFloat("pitch", s_grid_quantity, true);

    if (s_cursor) {
        s_cursor->SetPosition(ImGui::GetMousePos().x, ImGui::GetMousePos().y);   
        s_cursor->Render(static_cast<float>(System::Window::s_width * 2), static_cast<float>(System::Window::s_height * 2));
    }
     
    //Renderer::CreateFrameBuffer();

}



//---------------------


void GUI::Close()
{
    
    s_running = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    Editor::Log("GUI exited");

}


//---------------------


void GUI::ShowOptionsInit()
{

    #ifdef IMGUI_HAS_VIEWPORT
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
    #else
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    #endif

    bool pOpen = true;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Welcome", &pOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    // ImGuiStyle& style = ImGui::GetStyle();
    // float width = 0.0f;
    // width += ImGui::CalcTextSize("New").x;
    // width += style.ItemSpacing.x;
    // width += 150.0f;
    // width += style.ItemSpacing.x;
    // width += ImGui::CalcTextSize("Open").x;

    // AlignForWidth(width);

    if (ImGui::Button("New", ImVec2(System::Window::s_width, 0.0f))) {
 
        if (Editor::events.NewProject())
            s_show_init = false;
    }

    if (ImGui::Button("Open", ImVec2(System::Window::s_width, 0.0f))) {
        if (Editor::events.Open())
            s_show_init = false;
    }

    //render backsplash image to framebuffer

    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    Renderer::RescaleFrameBuffer(window_width, window_height);

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        (void*)Graphics::Texture2D::Get("icon large").ID,
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + window_width, pos.y + window_height),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    Renderer::BindFrameBuffer();

    Renderer::UnbindFrameBuffer();

    ImGui::SetCursorPos((ImVec2((ImGui::GetWindowSize().x * 0.5f) - 270, (ImGui::GetWindowSize().y * 0.5f) - 230)));


    ImGui::End();
    ImGui::PopStyleVar();

}


//---------------------


void GUI::ShowOptionsQuit()
{
    ImGui::Text("Do You Want To Quit?");

    if (ImGui::MenuItem("Yes")) {
        s_show_quit = false;
        glfwSetWindowShouldClose(System::Window::s_instance, true);
    }

    if (ImGui::MenuItem("No"))
        s_show_quit = false;
}



//--------------------- save and quit or save prior to build


void GUI::ShowOptionsSave(bool quit)
{

    ImGui::Text("Do You Want To Save?");

    if (ImGui::MenuItem("Yes")) 
        if(Editor::events.SaveScene()) 
        {
            Editor::events.buildFlag = false;

            if (quit) 
                Editor::events.exitFlag = true;
            else 
                Editor::events.BuildAndRun();
        }

    if (ImGui::MenuItem("No")) 
    {

        Editor::events.buildFlag = false;
        
        if (quit)
            Editor::events.exitFlag = true;
        else 
            Editor::events.BuildAndRun();
    }
       
}


//-------------------------------- mouse scroll


void GUI::scroll_callback(GLFWwindow* window, double xOffset, double yOffset) 
{

    if (!ImGui::IsAnyItemHovered()) 
    {

        //zoom camera
 
        float zoom = Editor::game->camera->GetZoom();

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            Editor::game->camera->SetZoom(yOffset > -1 ? zoom += 0.1 : zoom -= 0.1);
        
        //position camera

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            Editor::game->camera->SetPosition({ Editor::game->camera->position.x + yOffset * 10, Editor::game->camera->position.y });
        
        else 
            Editor::game->camera->SetPosition({ Editor::game->camera->position.x, Editor::game->camera->position.y + yOffset * 10 });

        if (xOffset != 0)
            Editor::game->camera->SetPosition({ Editor::game->camera->position.x + xOffset * 10, Editor::game->camera->position.y });
    }
}

