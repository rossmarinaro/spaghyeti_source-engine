#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"
#include "../../window/renderer.h"

#if DEVELOPMENT == 1 && STANDALONE == 1
    #include "../../../build/sdk/include/displayInfo.h"
    DisplayInfo* displayInfo;
#endif

using namespace System;      



void Game::Flush(bool removeBehaviors)
{

    LOG("Scene: " + currentScene->key + " stopped.");
    
    inputs->ResetControls();

    if (Application::isMultiThreaded && Application::eventPool) {
        delete Application::eventPool; 
        Application::eventPool = nullptr;
    }

    if (removeBehaviors)
       currentScene->behaviors.clear();

    maps->layers.clear();
    currentScene->UI.clear();   
    currentScene->entities.clear();
    time->timed_events.clear();
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

    glfwSetWindowTitle(Renderer::GLFW_window_instance, Application::name.c_str());

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

        #if STANDALONE == 1

            //refresh physics

            game->physics->ClearBodies();  

            //game->physics->GetWorld().SetContactListener(&game->physics->collisions);

        #endif

        //clear entities if applicable (after first initialization)

        if (game->currentScene) 
        {
            if (game->currentScene->key == game->scenes[0]->key) {
               Resources::Manager::Clear(false);  
               cachedScenes.clear();
            }

            game->Flush(game->currentScene->key == key);

        }   

        //assign / load current scene
        
        game->currentScene = *it;

        if (Application::isMultiThreaded)
            Application::eventPool = new EventPool(THREAD_COUNT);

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

        //clear behaviors from other scenes

        if (loadMap)
            for (const auto& scene : game->scenes)
                if (scene != game->currentScene) 
                    scene->behaviors.clear();

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

    Flush();

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

    inputs->ProcessInput();

    //entity render queue

    for (const auto& entity : currentScene->entities)
        if ((entity.get() && entity)) 
        {
            if (entity->cull && Entity::s_cullPosition)
            { 
                float width = System::Window::s_scaleWidth;

                if (!camera->InBounds() && Entity::s_cullPosition->x > System::Window::s_scaleWidth)
                    width = width + (width / 2);

                entity->renderable = (entity->position.x > Entity::s_cullPosition->x && (entity->position.x < Entity::s_cullPosition->x + width) * entity->scrollFactor.x) ||
                                     (entity->position.x < Entity::s_cullPosition->x && (entity->position.x > Entity::s_cullPosition->x - width) * entity->scrollFactor.x);
            }

            if (entity->renderable)
                entity->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
        }
 
    //UI render queue

    for (const auto& UI : currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
        
    //vignette overlay

    if(currentScene->vignette) {
        currentScene->vignette->SetSize(Window::s_scaleWidth * 4, Window::s_scaleHeight * 4);
        currentScene->vignette->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
    }

    //depth sort

    std::sort(currentScene->entities.begin(), currentScene->entities.end(), [](auto a, auto b){ return a->depth < b->depth; });
    std::sort(currentScene->UI.begin(), currentScene->UI.end(), [](auto a, auto b){ return a->depth < b->depth; });

    #if DEVELOPMENT == 1 

        if (physics->enableDebug) 
        {
            physics->GetWorld().DebugDraw();
            physics->debug->Flush();

            #if STANDALONE == 1
                displayInfo->Update(&m_context);
            #endif
        }

    #endif

    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : currentScene->behaviors)
        if (behavior.get() && behavior->active) 
        {
            if ((!currentScene->IsPaused() && behavior->layer == 0)) 
                behavior->Update();  
                
            else if (behavior->layer == 1)
                behavior->Update();
        }

    //physics

    if (!currentScene->IsPaused())
        physics->Update();

    //camera
 
    camera->Update();

    //cleanup events

    for (auto it = time->timed_events.begin(); it != time->timed_events.end(); ++it)
        if (it != time->timed_events.end())
        {
            auto event = *it;

            if (!event->active) {
                it = time->timed_events.erase(std::move(it));
                --it;
            } 
                        
        }

    //execute timed events on the main thread

    for (auto& event : time->timed_events)
        if (event->active && (time->GetSeconds() - event->time_initiated) >= (float)(event->delay / 1000.0f)) 
        {
            //refresh timed event token

            if (event->repeat > 0 || event->repeat == -1) {
                
               if (event->repeat != -1)
                  event->repeat--;

                event->time_initiated = time->GetSeconds(); 
            }

            else 
                event->active = false;  

            //fire callback

            event->callback();

        }


}


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

    const std::string ID = entity->ID;

    auto it = std::find(Application::game->currentScene->entities.begin(), Application::game->currentScene->entities.end(), entity);

    if (it != Application::game->currentScene->entities.end()) {
        it = Application::game->currentScene->entities.erase(std::move(it));
        --it;
    }

    else {
        auto UI_it = std::find(Application::game->currentScene->UI.begin(), Application::game->currentScene->UI.end(), entity);

        if (UI_it != Application::game->currentScene->UI.end()) {
            UI_it = Application::game->currentScene->UI.erase(std::move(UI_it));
            --UI_it;
        }
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

    if (entity.unique())
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

    ts->ReadSpritesheetData(); 

    Application::game->currentScene->entities.emplace_back(ts);

    return ts;

}


//----------------------------- text


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y, const std::string& font, int layer)
{
    auto text = std::make_shared<Text>(content, x, y, font); 

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


std::shared_ptr<Entity> Game::CreateEntity(int type, int layer)
{
    auto entity = std::make_shared<Entity>(type);

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(entity);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(entity);

    return entity;
} 



