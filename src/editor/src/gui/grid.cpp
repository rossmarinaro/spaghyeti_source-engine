#include "./gui.h"
#include "../../../../build/include/app.h"

//using namespace Editor;

void GUI::CreateGrid()
{

    static const char* checker_vertex = 
                    
        "#version 330 core\n"

        "out vec2 uv;\n"

        "void main()\n"
        "{\n"           
        "   float size = 10.0;\n"
        "   vec2 pos = floor(fragColor / size);\n"
        "   position = pos;\n"
        "   gl_Position = vec4(1.0, 1.0, 0.0, 1.0);\n" 
        "}\n"; 


    static const char* checker_fragment =  

        "#version 330 core\n"

        "in vec2 position;\n"
        "out vec4 color;\n"

        "void main()\n"
        "{\n"    
        "   float mask = mod(position.x + mod(position.y, 2.0), 2.0);\n"
        "   color = mask * vec4(1.0, 1.0, 1.0, 1.0);\n" 
        "}\n"; 

    Shader::Load("checkers", checker_vertex, checker_fragment, nullptr); 

    grid = std::make_unique<Graphics::Rectangle>(0, 0, System::Window::m_width, System::Window::m_height);
    grid->m_graphicsShader = Shader::GetShader("checkers");

}


//----------------------------


void GUI::RenderGrid()
{

    if (grid)
        grid->Render();

    // static ImVector<ImVec2> points;
    // static ImVec2 scrolling(0.0f, 0.0f);
    // static bool opt_enable_grid = true;
    // static bool opt_enable_context_menu = true;
    // static bool adding_line = false;

    // ImGui::Checkbox("Enable grid", &opt_enable_grid);
    // ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
    // ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

    // ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    // ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    // if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    // if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    // ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // // Draw border and background color
    // ImGuiIO& io = ImGui::GetIO();
    // ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    // draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // // This will catch our interactions
    // ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    // const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    // const bool is_active = ImGui::IsItemActive();   // Held
    // const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    // const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    // // Add first and second point
    // if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    // {
    //     points.push_back(mouse_pos_in_canvas);
    //     points.push_back(mouse_pos_in_canvas);
    //     adding_line = true;
    // }
    // if (adding_line)
    // {
    //     points.back() = mouse_pos_in_canvas;
    //     if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    //         adding_line = false;
    // }

    // // Pan (we use a zero mouse threshold when there's no context menu)
    // // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    // const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    // if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    // {
    //     scrolling.x += io.MouseDelta.x;
    //     scrolling.y += io.MouseDelta.y;
    // }

    // // Context menu (under default mouse threshold)
    // // ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    // // if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
    // //     ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    // if (ImGui::BeginPopup("context"))
    // {
    //     if (adding_line)
    //         points.resize(points.size() - 2);
    //     adding_line = false;
    //     if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
    //     if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
    //     ImGui::EndPopup();
    // }

    // // // Draw grid + all lines in the canvas
    // draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    // if (opt_enable_grid)
    // {
    //     const float GRID_STEP = 64.0f;
    //     for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
    //     for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    // }
    // for (int n = 0; n < points.Size; n += 2)
    //     draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
    // draw_list->PopClipRect();
}
