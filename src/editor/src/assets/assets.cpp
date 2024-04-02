#include "./assets.h"
#include "../../../../build/include/app.h"

using namespace editor;

//---------------------------


void AssetManager::LoadAsset(const std::string& asset, const std::string& path)
{

    const std::string folder = GetFolder(asset),
                      texture = GetThumbnail(asset),
                      key = "\"" + asset + "\"",
                      developmentPath = "\"" + path + "resources\\assets" + folder + asset + "\"",
                      productionPath = "\"" + path + "build\\assets\\" + asset + "\"";

    loadedAssets.insert({ key, developmentPath });
    productionAssets.insert({ key, productionPath });

}

//----------------------------


void AssetManager::SetIcon(const std::string& key)
{
    //splash image

    #ifndef __EMSCRIPTEN__

        GLFWimage image; 
        
        image.width = 70;
        image.height = 70;
        image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData(key)));
    
        glfwSetWindowIcon(System::Window::s_instance, 1, &image);

    #endif
}


//----------------------------


std::string AssetManager::GetFolder(const std::string& asset)
{
    std::string folder;

    if (System::Utils::GetFileType(asset) == "image") 
        folder = "\\images\\";

    if (System::Utils::GetFileType(asset) == "audio") 
        folder = "\\audio\\";

    if (System::Utils::GetFileType(asset) == "data") 
        folder = "\\data\\";

    return folder;
}


//-----------------------------


std::string AssetManager::GetThumbnail(const std::string& asset)
{
    std::string texture;

    if (System::Utils::GetFileType(asset) == "image") 
        texture = asset; 
    
    if (System::Utils::GetFileType(asset) == "audio") 
        texture = "audio src";
    
    if (System::Utils::GetFileType(asset) == "data") 
        texture = "data src";
    

    return texture;
}


//---------------------------------


void AssetManager::SavePrefab(void* node)
{

    // auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

    // //frames

    // json frames = json::array();

    // for (int i = 0; i < sn->frame; ++i)
    //     frames.push_back({
    //         { "frame x", sn->frameBuf1.size() ? sn->frameBuf1[i] : 0 },
    //         { "frame y", sn->frameBuf2.size() ? sn->frameBuf2[i] : 0 },
    //         { "frame width", sn->frameBuf3.size() ? sn->frameBuf3[i] : 0 },
    //         { "frame height", sn->frameBuf4.size() ? sn->frameBuf4[i] : 0 }
    //     });

    // //animations

    // json animations = json::array();

    // for (int i = 0; i < sn->anim; ++i)
    //     if (sn->animBuf1.size())
    //         animations.push_back({
    //             { "key", sn->animBuf1[i].s },
    //             { "start", sn->animBuf2[i] },
    //             { "end", sn->animBuf3[i] }
    //         });

    // //physics bodies

    // json bodies = json::array();

    // for (int i = 0; i < sn->bodies.size(); ++i)
    //     bodies.push_back({
    //         { "type", sn->bodies[i].second },
    //         { "body_width", sn->body_width.size() ? sn->body_width[i] : 0 },
    //         { "body_height", sn->body_height.size() ? sn->body_height[i] : 0 },
    //         { "bodyX", sn->bodyX.size() ? sn->bodyX[i] : 0 },
    //         { "bodyY", sn->bodyY.size() ? sn->bodyY[i] : 0 },
    //         { "pointer", sn->body_pointer.size() ? sn->body_pointer[i] : 0 },
    //         { "sensor", sn->is_sensor.size() ? sn->is_sensor[i].b : false }
    //     });


    // //settings

    // sprites.push_back({
    //     { "ID", node->m_ID }, 
    //     { "name", node->m_name },
    //     { "U1", sn->spriteHandle->m_texture.U1 },
    //     { "V1", sn->spriteHandle->m_texture.V1 },
    //     { "U2", sn->spriteHandle->m_texture.U2 },
    //     { "V2", sn->spriteHandle->m_texture.V2 },
    //     { "currentTexture", sn->spriteHandle->m_key },
    //     { "m_tint", {
    //             { "x", sn->spriteHandle->m_tint.x },
    //             { "y", sn->spriteHandle->m_tint.y },
    //             { "z", sn->spriteHandle->m_tint.z }
    //         }
    //     },
    //     { "m_alpha", sn->spriteHandle->m_alpha },
    //     { "positionX", sn->positionX },
    //     { "positionY", sn->positionY },
    //     { "rotation", sn->rotation },
    //     { "scaleX", sn->scaleX },
    //     { "scaleY", sn->scaleY },
    //     { "flipX", sn->flippedX },
    //     { "flipY", sn->flippedY },
    //     { "depth", sn->depth },
    //     { "lock", sn->lock_in_place },
    //     { "frames", frames },
    //     { "components", {
    //             { "physics", {
    //                     { "exists", sn->HasComponent("Physics") },
    //                     { "bodies", bodies },
    //                     { "friction", sn->friction },
    //                     { "restitution", sn->restitution },
    //                     { "density", sn->density }
    //                 }
    //             },
    //             { "animator", {
    //                     { "exists", sn->HasComponent("Animator") },
    //                     { "animations", animations }
    //                 }
    //             },
    //             { "script", {
    //                     { "exists", sn->HasComponent("Script") },
    //                     { "scripts", scripts }
    //                 }
    //             },
    //             { "shader", {
    //                     { "exists", sn->HasComponent("Shader") },
    //                     { "shaders", shader }
    //                 }
    //             }
    //         }
    //     }
    // });
}


