#include "../../../../build/sdk/include/app.h"
#include "./gui.h"
#include "../editor.h"
#include "../assets/assets.h"

#include "../assets/embedded/images/data.hpp"
#include "../assets/embedded/images/audio.hpp"
#include "../assets/embedded/images/icon_small.hpp"
#include "../assets/embedded/images/icon_large.hpp"
#include "../assets/embedded/images/folder.hpp"
#include "../assets/embedded/images/text.hpp"

using namespace editor;

//------------------------------------- launch GUI

GUI::GUI()
{
    s_self = this;

    s_self->running = true;
    s_self->show_init = true;
    s_self->show_quit = false;
    s_self->show_grid = false;

    //Poll and handle events (inputs, window resize, etc.)
    //You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    //- When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    //- When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    //Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

    Editor::Log("IMGui Version: " + (std::string)ImGui::GetVersion());

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

    ImGui_ImplGlfw_InitForOpenGL(System::Renderer::GLFW_window_instance, true);
    ImGui_ImplOpenGL3_Init(System::Window::s_glsl_version);

    static constexpr const char* checker_vertex = 

        "#version 330 core\n"

        "layout (location = 0) in vec2 vert;\n"

        "out vec2 position;\n"

        "void main()\n"
        "{\n"    
            "gl_Position = vec4(vert.xy, 0.0, 1.0);\n" 
        "}"; 


    static constexpr const char* checker_fragment =  

        "#version 330 core\n"

        "precision mediump float;\n"

        "uniform float alpha;\n"
        "uniform float pitch;\n"
        "out vec4 color;\n"

        "void main()\n" 
        "{\n"    

            "vec2 pitch = vec2(pitch, pitch);\n"

            "if (mod(gl_FragCoord.x, pitch[0]) < 1.0 || mod(gl_FragCoord.y, pitch[1]) < 1.0) \n"
                "color = vec4(0.25, 0.25, 0.25, alpha);\n"
            "else \n"
                "color = vec4(0.0);\n"
        "}";


    Graphics::Shader::Load("grid", checker_vertex, checker_fragment); 

    s_self->grid = std::make_unique<Geometry>(-10, -10, 1500, 1500);
    s_self->grid->SetShader("grid");
    s_self->grid_quantity = 20.0f;

    //load embedded assets
 
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "icon small", icon_small_png, icon_small_png_len); 
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "editor logo", icon_large_png, icon_large_png_len);
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "audio src", audio_png, audio_png_len); 
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "data src", data_png, data_png_len);
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "folder src", folder_png, folder_png_len);
    System::Resources::Manager::LoadRaw(System::Resources::Manager::IMAGE, "text src", text_png, text_png_len);

    System::Resources::Manager::RegisterTextures();

    glfwSetScrollCallback(System::Renderer::GLFW_window_instance, scroll_callback); 

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
    if (!running)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    if (show_init)
        ShowOptionsInit();

    else
        RenderDockSpace();

    if (show_quit)
        ShowOptionsQuit();

    if (Editor::Get()->events->buildFlag)
        ShowOptionsSave(false);

    else if (Editor::Get()->events->saveFlag)
        ShowOptionsSave(true);

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
}


//---------------------


GUI::~GUI()
{
    running = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    Editor::Log("GUI exited.");

    Editor::ShutDown();
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

    if (ImGui::Button("New", ImVec2(System::Window::s_width, 0.0f))) {
 
        if (Editor::Get()->events->NewProject())
            show_init = false;
    }

    if (ImGui::Button("Open", ImVec2(System::Window::s_width, 0.0f))) {
        if (Editor::Get()->events->OpenProject())
            show_init = false;
    }
 
    //render backsplash image to framebuffer

    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;
 
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage((void*)Graphics::Texture2D::Get("editor logo").ID, ImVec2(pos.x, pos.y), ImVec2(pos.x + window_width, pos.y + window_height));

    ImGui::SetCursorPos((ImVec2((ImGui::GetWindowSize().x * 0.5f) - 270, (ImGui::GetWindowSize().y * 0.5f) - 230)));

    ImGui::End();
    ImGui::PopStyleVar();

}


//---------------------


void GUI::ShowOptionsQuit()
{
    ImGui::Text("Do You Want To Quit?");

    if (ImGui::MenuItem("Yes")) {
        show_quit = false;
        glfwSetWindowShouldClose(System::Renderer::GLFW_window_instance, true);
    }

    if (ImGui::MenuItem("No"))
        show_quit = false;
}



//--------------------- save and quit or save prior to build


void GUI::ShowOptionsSave(bool quit)
{
    auto session = Editor::Get();

    ImGui::Text("Do You Want To Save?");

    if (ImGui::MenuItem("Yes")) 
        if(session->events->SaveScene()) 
        {
            session->events->buildFlag = false;

            if (quit) 
                session->events->exitFlag = true;
            else 
                session->events->BuildAndRun();
        }

    if (ImGui::MenuItem("No")) {

        session->events->buildFlag = false;
        
        if (quit)
            session->events->exitFlag = true;
        else 
            session->events->BuildAndRun();
    }
       
}


//-------------------------------- mouse scroll


void GUI::scroll_callback(GLFWwindow* window, double xOffset, double yOffset) 
{
    if (!ImGui::IsAnyItemHovered()) 
    {
        auto session = Editor::Get();

        //zoom camera
 
        const float zoom = *session->game->camera->GetZoom();

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            session->game->camera->SetZoom(yOffset > -1 ? zoom + 0.1 : zoom - 0.1);
        
        //position camera

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            session->game->camera->SetPosition({ session->game->camera->GetPosition()->x + yOffset * 10, session->game->camera->GetPosition()->y });
        
        else 
            session->game->camera->SetPosition({ session->game->camera->GetPosition()->x, session->game->camera->GetPosition()->y + yOffset * 10 });

        if (xOffset != 0)
            session->game->camera->SetPosition({ session->game->camera->GetPosition()->x + xOffset * 10, session->game->camera->GetPosition()->y });
    }
}

//----------------------- render node types in gui


void GUI::RenderNodes()
{
    if (!running)
        return;

    if (ImGui::BeginMenu("New Node"))
    {
        if (ImGui::MenuItem("Load Prefab")) 
        {
            if (AssetManager::LoadPrefab())
                Editor::Log("Prefab loaded.");
            else 
                Editor::Log("There was a problem loading prefab.");
        } 

        ImGui::Separator();

        if (ImGui::MenuItem("Group")) 
            Node::Make<GroupNode>(true);

        if (ImGui::MenuItem("Empty"))
            Node::Make<EmptyNode>(true); 

        if (ImGui::MenuItem("Audio"))
            Node::Make<AudioNode>(true); 

        if (ImGui::MenuItem("Text"))
            Node::Make<TextNode>(true); 

        if (ImGui::MenuItem("Sprite"))
            Node::Make<SpriteNode>(true); 

        if (ImGui::MenuItem("Tilemap"))
            Node::Make<TilemapNode>(true); 

        if (ImGui::MenuItem("Spawner"))
            Node::Make<SpawnerNode>(true); 

        ImGui::EndMenu();
    }

    //render active nodes
        
    if (Node::nodes.size())
        for (const auto& node : Node::nodes)
            if (node && node->active) { 
                node->Update();  
                node->Render();  
            }
}

