#include "../../../build/sdk/include/app.h"

#if DEVELOPMENT == 1 && STANDALONE == 1

    #include "../../../build/sdk/include/displayInfo.h"
    DisplayInfo* displayInfo;

#endif

using namespace System;      


void Game::Flush(bool removeMap)
{

    LOG("Scene: " + currentScene->key + " stopped.");
    
    inputs->ResetControls();

    if (Application::eventPool) {
        delete Application::eventPool; 
        Application::eventPool = nullptr;
    }
    
    if (removeMap) {
        maps->layers.clear();
        currentScene->entities.clear();
    }
    
    else
        currentScene->entities.erase(std::remove_if(currentScene->entities.begin(), currentScene->entities.end(), [](const auto e) { return strcmp(e->type, "tile") != 0; }), currentScene->entities.end());

    currentScene->UI.clear();
    currentScene->behaviors.clear();
}


//------------------------------


Scene* Game::GetScene(const std::string& key) 
{
    if (!key.length())
        return Application::game->currentScene;

    auto it = std::find_if(Application::game->scenes.begin(), Application::game->scenes.end(), [&](auto scene) { return scene->key == key; });

    if (it == Application::game->scenes.end()) 
        return *it;

    return nullptr;
}

  
//------------------------- backend game layer functionality   


Game::Game()
{

    //game components

    inputs = new Inputs;
    time = new Time;
    camera = new Camera;
    physics = new Physics;

    maps = new MapManager;

    //scene context handles

    m_context = { inputs, camera, physics, time };

}


//---------------------------- boot entry scene


void Game::Boot()   
{     
    LOG("Game: " + Application::name + " initializing.");

    currentScene = nullptr;

    text->Init(); 

    //preload / run game layer

    StartScene(scenes[0]->key, true); 

    glfwSetWindowTitle(Window::s_instance, Application::name.c_str());

    inputs->ResetControls();

    physics->GetWorld().SetContactListener(&physics->collisions);
    
    //physics listener and debug

    #if DEVELOPMENT == 1 

        physics->debug = new DebugDraw;
	    physics->GetWorld().SetDebugDraw(physics->debug); 

        #if STANDALONE == 1
            displayInfo = new DisplayInfo; 
            physics->enableDebug = false;
        #else
            physics->enableDebug = true;
        #endif

    #endif

}


//-----------------------------


void Game::StartScene(const std::string& key, bool loadMap) 
{

    Game* game = Application::game; 

    game->m_gameState = false;
 
    //find loaded scene

    auto it = std::find_if(game->scenes.begin(), game->scenes.end(), [&](Scene* scene) { return scene->key == key; });

    if (it != game->scenes.end())
    {

        //clear entities if applicable (after first initialization)

        if (game->currentScene) 
        {
            if (game->currentScene->key == game->scenes[0]->key) {
               Resources::Manager::Clear(false);  
               cachedScenes.clear();
            }

            game->Flush(game->currentScene->key != key);

        }   

        //assign / load current scene
        
        game->currentScene = *it;

        Application::eventPool = new EventPool(THREAD_COUNT);

        #if STANDALONE == 1

            //refresh physics

            game->physics->ClearBodies();  

            #if DEVELOPMENT == 1 

                delete game->physics->debug;
                game->physics->debug = nullptr;

                delete displayInfo;
                displayInfo = nullptr;

                game->physics->debug = new DebugDraw;
                game->physics->GetWorld().SetDebugDraw(game->physics->debug);

                displayInfo = new DisplayInfo;

            #endif

            game->physics->GetWorld().SetContactListener(&game->physics->collisions);

        #endif

        if (std::find(cachedScenes.begin(), cachedScenes.end(), game->currentScene->key) == cachedScenes.end()) {
            game->currentScene->Preload();
            cachedScenes.emplace_back(game->currentScene->key);
        }

        game->currentScene->vignette = std::make_unique<Geometry>(0.0f, -50.0f, 0.0f, 0.0f);
        game->currentScene->vignette->SetTint({ 0.0f, 0.0f, 0.0f });
        game->currentScene->vignette->SetAlpha(0.0f);
        game->currentScene->vignette->SetScrollFactor({ 0.0f, 1.0f });

        game->currentScene->Run(loadMap);  

        game->m_gameState = true;

        const std::string state = loadMap ? " started" : " restarted.";

        LOG("Scene: " + key + state);

        return;
    }

    else 
        LOG("Scene: key not found.");

}



//-----------------------------


void Game::Exit() 
{

    m_gameState = false;

    Flush(true);

    for (auto& scene : scenes) {
        delete scene;
        scene = nullptr;
    }

    scenes.clear();   
    cachedScenes.clear(); 
    text->ShutDown();

    #if DEVELOPMENT == 1 

       delete physics->debug;
       physics->debug = nullptr;

        #if STANDALONE == 1

            delete displayInfo;
            displayInfo = nullptr;

        #endif

    #endif

    delete camera;
    camera = nullptr;

    delete physics;
    physics = nullptr;

    delete time;
    time = nullptr;

    delete inputs;
    inputs = nullptr;

    delete maps;
    maps = nullptr;

    LOG("Game: exited.");

}


//----------------------------


void Game::UpdateFrame()
{

    if (!m_gameState)
        return;

    inputs->ProcessInput(Window::s_instance);

    if (!currentScene->IsPaused())
        physics->Update();

    //entity render queue

    for (const auto& entity : currentScene->entities)
        if ((entity.get() && entity) && entity.get()->renderable) 
            entity->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
           

    //vignette overlay

    if(currentScene->vignette) {
        currentScene->vignette->SetSize(Window::s_scaleWidth * 4, Window::s_scaleHeight * 4);
        currentScene->vignette->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
    }

    //UI render queue

    for (const auto& UI : currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    //depth sort

    std::sort(currentScene->entities.begin(), currentScene->entities.end(), [](auto a, auto b){ return a->depth < b->depth; });
    std::sort(currentScene->UI.begin(), currentScene->UI.end(), [](auto a, auto b){ return a->depth < b->depth; });

    #if DEVELOPMENT == 1 

        if (physics->enableDebug) 
        {
            physics->GetWorld().DebugDraw();
            physics->debug->Flush();

            #if STANDALONE == 1
                displayInfo->Update(m_context);
            #endif
        }

    #endif

    // auto inactive_behavior_it = std::find_if(currentScene->behaviors.begin(), currentScene->behaviors.end(), [](auto b) { return b->active == false; });

    // if (inactive_behavior_it != currentScene->behaviors.end()) {
    //     Application::game->currentScene->behaviors.erase(inactive_behavior_it);
    //     (*inactive_behavior_it).reset();
    // }

    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : currentScene->behaviors) 
        if (m_gameState && (behavior.get() && behavior) && behavior->active) 
        {
            if ((!currentScene->IsPaused() && behavior->layer == 0)) 
                behavior->Update();  
                
            else if (behavior->layer == 1)
                behavior->Update();
        }
    
}  


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

    const std::string ID = entity->ID;

    auto it = std::find(Application::game->currentScene->entities.begin(), Application::game->currentScene->entities.end(), entity);

    if (it != Application::game->currentScene->entities.end())
        Application::game->currentScene->entities.erase(it);

    else {
        auto UI_it = std::find(Application::game->currentScene->UI.begin(), Application::game->currentScene->UI.end(), entity);

        if (UI_it != Application::game->currentScene->UI.end())
            Application::game->currentScene->UI.erase(UI_it);
    }

    entity->renderable = false;

    entity->active = false;
    entity->alive = false;

    if (entity->IsSprite()) 
    {
        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) {
            for (const auto& body : sprite->bodies)
                Physics::DestroyBody(body.first); 

            sprite->bodies.clear();
        }
    }

    //remove from vector and disappear into the void

    if (!Application::game->currentScene->entities.size())
       entity.reset(); 

    //reset associated behavior if applicable

    auto behavior_it = std::find_if(Application::game->currentScene->behaviors.begin(), Application::game->currentScene->behaviors.end(), [&](auto b) { return b->ID == ID; });

    if (behavior_it != Application::game->currentScene->behaviors.end()) 
        (*behavior_it)->active = false;


} 


//----------------------------- entity containers for instantiation

//----------------------------- sprite


std::shared_ptr<Sprite> Game::CreateSprite(const std::string& key, float x, float y, int frame, float scale, int layer)
{

    auto sprite = std::make_shared<Sprite>(key, x, y, frame);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData(); 
    #endif

    sprite->SetScale(scale);

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(sprite);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(sprite); 

    return sprite;
}



//----------------------------- ui sprite


std::shared_ptr<Sprite> Game::CreateUI(const std::string& key, float x, float y, int frame)
{

    auto element = std::make_shared<Sprite>(key, glm::vec2(x, y));

    #if STANDALONE == 1
        element->ReadSpritesheetData(); 
    #endif
    
    element->SetFrame(frame);

    Application::game->currentScene->UI.emplace_back(element);

    return element;
}


//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{

    auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    ts->type = "tile";
    //ts->shader = Shader::Get("batch");
    ts->ReadSpritesheetData(); 

    Application::game->currentScene->entities.emplace_back(ts);

    return ts;

}


//----------------------------- debug text


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y, int layer)
{

    auto text = std::make_shared<Text>(content, x, y); 

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(text);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(text);

    return text;
}


//----------------------------- quad


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height, int layer, bool isStatic)
{
    auto geom = std::make_shared<Geometry>(x, y, width, height);

    if (isStatic)
        geom->isStatic = true;

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(geom);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(geom);

    return geom;
}


//----------------------------- base entity wrapper


std::shared_ptr<Entity> Game::CreateEntity(const std::string& type, int layer)
{
    auto entity = std::make_shared<Entity>(type.c_str());

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(entity);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(entity);

    return entity;
} 



