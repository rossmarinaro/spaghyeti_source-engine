#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"

#include "../../shared/renderer.h"
#include "./collisionManager.h"

#if DEVELOPMENT == 1 

    #if STANDALONE == 1
        #include "./displayInfo.h"
        static DisplayInfo* _displayInfo;
    #endif
    
    #include "./debug.h"

    static DebugDraw* _debug;
    static unsigned int _debug_flags = 0;

#endif

using namespace System;

static CollisionManager _collisions;

void Game::Flush(bool removeBehaviors)
{
    LOG("Scene: " + currentScene->key + " stopped.");

    inputs->ResetControls();

    if (Application::events->isMultiThreaded && Application::events->pool) {
        delete Application::events->pool;
        Application::events->pool = nullptr;
    }

    if (removeBehaviors)
        currentScene->behaviors.clear();

    maps->layers.clear();
    currentScene->UI.clear();
    currentScene->entities.clear();
    time->timed_events.clear();                  
    
    physics->ClearBodies(); 
    camera->Reset(); 

    //clear behaviors from other scenes

    for (const auto& scene : scenes) {
        for (auto& behavior : scene->behaviors)
            behavior.reset(); 
        
        scene->behaviors.clear();
    }          
}
 

//------------------------------


Scene* Game::GetScene(const std::string& key)
{
    if (!key.length())
        return Application::game->currentScene;

    const auto it = std::find_if(Application::game->scenes.begin(), Application::game->scenes.end(), [&](auto scene) { return scene->key == key; });

    if (it != Application::game->scenes.end())
        return *it;

    return nullptr;
}


//------------------------- backend game layer functionality


Game::Game()
{
    LOG("Game: initializing context...");

    //game components

    inputs = new Inputs;
    time = new Time;
    camera = new Camera;
    physics = new Physics;

    maps = new MapManager;

    //scene context handles

    m_context = { true, inputs, camera, physics, time };
}


//---------------------------- boot entry scene


void Game::Boot()
{
    if (!&m_context) {
        LOG("Game: Error - there was a problem initializing context.");
        return;
    }

    currentScene = nullptr;

    text->Init();

    LOG("Game: " + Application::name + " initialized.");
    
    Graphics::Shader::InitBaseShaders();
    
    //preload / run game layer

    glfwSetWindowTitle(Renderer::GLFW_window_instance, Application::name.c_str());

    inputs->ResetControls();

    const auto& world = static_cast<b2World*>(physics->GetWorld());

    StartScene(scenes[0]->key, true);

    //physics listener and debug

    #if DEVELOPMENT == 1

        _debug_flags += b2Draw::e_shapeBit;
        _debug_flags += b2Draw::e_jointBit;
        _debug_flags += b2Draw::e_aabbBit;
        _debug_flags += b2Draw::e_centerOfMassBit;

        _debug = new DebugDraw;

        if (world)
            world->SetDebugDraw(_debug);

        #if STANDALONE == 1
            _displayInfo = new DisplayInfo(&m_context);
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
    game->m_context.active = false;

    //find loaded scene

    const auto it = std::find_if(game->scenes.begin(), game->scenes.end(), [&](Scene* scene) { return scene->key == key; });

    if (it != game->scenes.end())
    {
        //clear entities if applicable (after first initialization)

        if (game->currentScene)
        {     
            if (game->currentScene->key == game->scenes[0]->key) {
                Resources::Manager::Clear(false);
                cachedScenes.clear();
            }

            game->Flush(game->currentScene->key == key);     
        }
        
        const auto& world = static_cast<b2World*>(game->physics->GetWorld());

        if (world)
            world->SetContactListener(&_collisions);

        game->currentScene = *it; 
            
        if (Application::events->isMultiThreaded)
            Application::events->pool = new Events::EventPool(THREAD_COUNT);

        if (std::find(cachedScenes.begin(), cachedScenes.end(), game->currentScene->key) == cachedScenes.end()) {
            LOG("Scene: " + key + " loading.");
            game->currentScene->Preload();
            cachedScenes.emplace_back(game->currentScene->key);
        }

        game->currentScene->vignette = std::make_unique<Geometry>(0.0f, -50.0f, 0.0f, 0.0f);
        game->currentScene->vignette->SetTint({ 0.0f, 0.0f, 0.0f });
        game->currentScene->vignette->SetAlpha(0.0f);
        game->currentScene->vignette->SetScrollFactor({ 0.0f, 1.0f });

        const std::string state = loadMap ? " started" : " restarted.";

        LOG("Scene: " + key + state);

        game->currentScene->Run(loadMap);

        game->m_context.active = true;       
        game->m_gameState = true;

        return;
    }

    LOG("Scene: key not found.");
}



//-----------------------------



void Game::Exit()
{
    m_gameState = false;

    Flush();

    for (auto &scene : scenes) {
        delete scene;
        scene = nullptr;
    }

    scenes.clear();
    cachedScenes.clear();
    text->ShutDown();

    #if DEVELOPMENT == 1

        delete _debug;
        _debug = nullptr;

        #if STANDALONE == 1
            delete _displayInfo;
            _displayInfo = nullptr;
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


void Game::SetCullPosition(Math::Vector2* position) 
{ 
    const auto scene = GetScene();

    if (scene)
        scene->cullPosition = position; 
}


//----------------------------


void Game::UpdateFrame()
{
    if (!m_gameState) 
        return;

    if (inputs)
        inputs->ProcessInput();

    Entity::s_rendered = 0;
 
    //entity render queue

    for (const auto& entity : currentScene->entities)
        if ((entity.get() && entity))
        {
            if (entity->cull && currentScene->cullPosition)
            {
                float width = Window::s_scaleWidth,
                      sfX = 1.0f;

                if (entity->scrollFactor.x < 1.0f) {
                    sfX = (1.0f - entity->scrollFactor.x) * 10.0f;

                    if (!camera->InBounds() && currentScene->cullPosition->x < Window::s_scaleWidth) 
                        sfX = (1.0f - entity->scrollFactor.x) * 2.0f;
                }

                if (entity->GetType() == Entity::SPRITE) {
                    auto sprite = std::static_pointer_cast<Sprite>(entity); 
                    sprite->renderable = ((sprite->position.x + sprite->texture.FrameWidth) * sfX > currentScene->cullPosition->x && (sprite->position.x + sprite->texture.FrameWidth) * sfX < (currentScene->cullPosition->x + sprite->texture.FrameWidth) * sfX + width) || 
                                         ((sprite->position.x - sprite->texture.FrameWidth) * sfX < currentScene->cullPosition->x && (sprite->position.x - sprite->texture.FrameWidth) * sfX > (currentScene->cullPosition->x - sprite->texture.FrameWidth) * sfX - width);
                }

                else
                    entity->renderable = (entity->position.x > currentScene->cullPosition->x && entity->position.x < currentScene->cullPosition->x + width) ||
                                         (entity->position.x < currentScene->cullPosition->x && entity->position.x > currentScene->cullPosition->x - width);
            }

            if (entity->renderable) {
                entity->Render();
                Entity::s_rendered++;
            } 
        }

    //UI render queue

    for (const auto& UI : currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render();

    //vignette overlay

    if (currentScene->vignette) {
        currentScene->vignette->SetSize(Window::s_scaleWidth * 4, Window::s_scaleHeight * 4);
        currentScene->vignette->Render();
    }

    //depth sort

    std::sort(currentScene->entities.begin(), currentScene->entities.end(), [](auto a, auto b) { return a->depth < b->depth; });
    std::sort(currentScene->UI.begin(), currentScene->UI.end(), [](auto a, auto b) { return a->depth < b->depth; });      

    #if DEVELOPMENT == 1

        if (physics && physics->enableDebug)
        {
            static_cast<b2World*>(physics->GetWorld())->DebugDraw();
            _debug->SetFlags(_debug_flags);
            _debug->Flush();

            #if STANDALONE == 1
                _displayInfo->Update(&m_context);
            #endif
        }

    #endif

    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : currentScene->behaviors)
        if (behavior.get() && behavior->active) {
            if ((!currentScene->IsPaused() && behavior->layer == 0))
                behavior->Update();

            else if (behavior->layer == 1)
                behavior->Update();
        }

    //physics

    if (physics && !currentScene->IsPaused())
        physics->Update();

    //camera

    if (camera)
        camera->Update();

    //cleanup events

    for (auto it = time->timed_events.begin(); it != time->timed_events.end(); ++it)
        if (it != time->timed_events.end()) {
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

            if (event->repeat > 0 || event->repeat == -1)
            {
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

        if (UI_it != Application::game->currentScene->UI.end())
        {
            UI_it = Application::game->currentScene->UI.erase(std::move(UI_it));
            --UI_it;
        }
    }

    entity->renderable = false;
    entity->active = false;
    entity->alive = false;

    if (entity->IsSprite())
    {
        const auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size())
        {
            for (const auto &body : sprite->bodies)
                Physics::DestroyBody(body.first);

            sprite->bodies.clear();
        }
    }

    // remove from vector and disappear into the void

    if (entity.unique())
        entity.reset();

    // reset associated behavior if applicable

    auto behavior_it = std::find_if(Application::game->currentScene->behaviors.begin(), Application::game->currentScene->behaviors.end(), [&](auto b)
                                    { return b->ID == ID; });

    if (behavior_it != Application::game->currentScene->behaviors.end())
        (*behavior_it)->active = false;
}



/* entity containers for instantiation */

//----------------------------- sprite

std::shared_ptr<Sprite> Game::CreateSprite(const std::string &key, float x, float y, int frame, float scale, int layer)
{

    const auto sprite = std::make_shared<Sprite>(key, x, y);

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(sprite);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(sprite);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData();
        sprite->SetFrame(frame);
    #endif

    sprite->SetScale(scale);

    return sprite;
}


//----------------------------- ui sprite


std::shared_ptr<Sprite> Game::CreateUI(const std::string &key, float x, float y, int frame)
{
    const Math::Vector2 pos = { x, y };

    const auto element = std::make_shared<Sprite>(key, pos);

    Application::game->currentScene->UI.emplace_back(element);

    #if STANDALONE == 1
        element->ReadSpritesheetData();
    #endif

    element->SetFrame(frame);

    return element;
}


//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string &key, float x, float y, int frame)
{

    const auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    Application::game->currentScene->entities.emplace_back(ts);

    ts->ReadSpritesheetData();

    return ts;
}


//----------------------------- text


std::shared_ptr<Text> Game::CreateText(const std::string &content, float x, float y, const std::string &font, int layer)
{
    const auto text = std::make_shared<Text>(content, x, y, font);

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(text);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(text);

    return text;
}


//----------------------------- quad


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height, int layer, bool isStatic)
{
    const auto geom = std::make_shared<Geometry>(x, y, width, height);

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
    const auto entity = std::make_shared<Entity>(type);

    if (layer == 1)
        Application::game->currentScene->entities.emplace_back(entity);

    if (layer == 2)
        Application::game->currentScene->UI.emplace_back(entity);

    return entity;
}
