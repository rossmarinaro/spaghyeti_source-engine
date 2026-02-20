#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"

#include "../../shared/renderer.h"
#include "./collisionManager.h"
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/matrix_transform.hpp"
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

    s_spawn_count = 0;

    if (removeBehaviors)
        currentScene->behaviors.clear();

    maps->layers.clear();
    currentScene->UI.clear();
    currentScene->entities.clear();
    currentScene->spawns.clear();
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
    s_spawn_count = 0;

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
    
    Graphics::Texture2D::InitBaseTexture();
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
        game->currentScene->vignette->SetScrollFactor({ 0.0f, 0.0f });

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
        scene->cameraTarget = position; 
}


//----------------------------


void Game::UpdateFrame()
{
    if (!m_gameState) 
        return;

    if (inputs)
        inputs->ProcessInput();
        
    //spawn update

    #if STANDALONE == 1

        if (currentScene->cameraTarget)
            for (auto& spawn : currentScene->spawns)
            {
                if (Math::distanceBetween(currentScene->cameraTarget->x - spawn.posX) >= spawn.spawn_width * 1.5f && spawn.loop)  
                    spawn.can_create = true;

                //create / runtime instantiation of game objects

                if (spawn.can_create && 
                    currentScene->cameraTarget->x >= spawn.posX - spawn.spawn_width && currentScene->cameraTarget->x <= spawn.posX + spawn.spawn_width &&
                    currentScene->cameraTarget->y >= spawn.posY - spawn.spawn_height && currentScene->cameraTarget->y <= spawn.posY + spawn.spawn_height
                ) 
                {
                    if (std::find_if(GetScene()->entities.begin(), GetScene()->entities.end(), [&spawn](std::shared_ptr<Entity> e) { return e->name == spawn.index; }) != System::Game::GetScene()->entities.end()) 
                        continue;

                    spawn.can_create = false; 

                    std::shared_ptr<Entity> entity;

                    switch (spawn.type) 
                    {
                        case Entity::SPRITE: 

                            entity = CreateSprite(spawn.filename, spawn.posX, spawn.posY, 0, 1.0f, 1, true); 

                            //apply physics body to spawned entity

                            if (spawn.body.exist) 
                            {
                                const auto sprite = std::static_pointer_cast<Sprite>(entity);

                                sprite->AddBody(Physics::CreateBody(
                                    spawn.body.type, 
                                    spawn.body.shape, 
                                    spawn.posX, 
                                    spawn.posY, 
                                    spawn.body.w, 
                                    spawn.body.h, 
                                    spawn.body.is_sensor, 
                                    1,                  
                                    spawn.body.density, 
                                    spawn.body.friction, 
                                    spawn.body.restitution
                                ), { 
                                    spawn.body.xOff, 
                                    spawn.body.yOff, 
                                    spawn.body.w, 
                                    spawn.body.h, 
                                }); 
                            }
                            
                        break;
                        case Entity::GEOMETRY: 
                            entity = CreateGeom(spawn.posX, spawn.posY, spawn.width, spawn.height, 1, false, true); 
                        break;
                        default: LOG("Spawner: failed to spawn, invalid enum type."); break;
                    }
                    
                    if (entity) {
                        entity->SetAlpha(spawn.alpha);
                        entity->SetTint(spawn.tint);
                        entity->SetName(spawn.index);
                    }
                }
            
                //remove

                if (
                    currentScene->cameraTarget->x < spawn.posX - (Window::s_scaleWidth * 2) ||
                    currentScene->cameraTarget->x > spawn.posX + (Window::s_scaleWidth * 2) || 
                    currentScene->cameraTarget->y < spawn.posY - (Window::s_scaleHeight * 2) ||
                    currentScene->cameraTarget->y > spawn.posY + (Window::s_scaleHeight * 2) 
                )
                {   
                    std::shared_ptr<Entity> entity;  

                    if (spawn.type == Entity::SPRITE)
                        entity = GetScene()->GetEntity<Sprite>(spawn.index);

                    else if (spawn.type == Entity::GEOMETRY)
                        entity = GetScene()->GetEntity<Geometry>(spawn.index);

                    if (entity) 
                    {
                        if (spawn.loop)
                            spawn.can_create = true;

                        for (auto& b : spawn.behaviors_attached)
                            b.second = false;
                        
                        const std::string id = entity->ID;

                        DestroyEntity(entity);

                        //remove associated behaviors

                        const auto behavior_it = std::find_if(GetScene()->behaviors.begin(), GetScene()->behaviors.end(), [&id](auto b)
                            { return b->ID == id; });

                        if (behavior_it != GetScene()->behaviors.end())
                        {
                            auto behavior = *behavior_it;
                            
                            behavior->active = false; 
                            behavior->Cleanup();
                            behavior.reset();
                        }
                    }
                }
            }

    #endif

    //entity render queue 

    Entity::s_rendered = 0;

    //sort opaque / transparent entities

    std::vector<std::shared_ptr<Entity>> opaque_entities, transparent_entities;

    for (const auto& entity : currentScene->entities) {
        if (entity->texture.IsOpaque())
            opaque_entities.emplace_back(entity);
        else 
            transparent_entities.emplace_back(entity);
    }

    std::sort(opaque_entities.begin(), opaque_entities.end(), [](auto a, auto b) { return a->depth < b->depth; }); //f-b <
    std::sort(transparent_entities.begin(), transparent_entities.end(), [](auto a, auto b) { return a->depth < b->depth; }); //b-f >

    //culling out of view sprites, pushing in-view sprites vertices to renderer, flush entities

    RenderEntities(opaque_entities);

    if (!opaque_entities.empty())
        Renderer::Flush();

    RenderEntities(transparent_entities);

    if (!transparent_entities.empty())
        Renderer::Flush(false);

    //UI render queue

    std::sort(currentScene->UI.begin(), currentScene->UI.end(), [](auto a, auto b) { return a->depth < b->depth; });

    for (const auto& UI : currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render();

    //flush UI

    if (!currentScene->UI.empty())
        Renderer::Flush(false); //fix

    //vignette overlay

    if (currentScene->vignette && currentScene->vignette->alpha >= 0.1f) {
        currentScene->vignette->SetSize(Window::s_scaleWidth * 4, Window::s_scaleHeight * 4);
        currentScene->vignette->Render(); 
        Renderer::Flush(false);    
    }     

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
            const auto event = *it;
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

            //fire event callback

            event->callback();
        } 
        
    //update camera

    currentScene->Update(); 

    //debug UI

    #if DEVELOPMENT == 1
        if (physics && physics->enableDebug) {
            static_cast<b2World*>(physics->GetWorld())->DebugDraw();
            _debug->SetFlags(_debug_flags);
            _debug->Flush();

            #if STANDALONE == 1
                _displayInfo->Update(&m_context);  
            #endif 
        } 
    #endif
  
}


//-----------------------------


bool Game::CheckEntityRenderable(std::shared_ptr<Entity>& entity) 
{
    //cull sprite and tile type entities out of view space

    const float width = entity->texture.FrameWidth,
                height = entity->texture.FrameHeight,
                camPosX = -camera->GetPosition()->x, 
                camPosY = -camera->GetPosition()->y,
                fx = (1.0f - entity->scrollFactor.x) + 1.0f,
                fy = (1.0f - entity->scrollFactor.y) + 1.0f,
                posX = entity->position.x * fx,
                posY = entity->position.y * fy,
                left = (camPosX - (Window::s_scaleWidth) / 2) * entity->scrollFactor.x, 
                right = (camPosX + (Window::s_scaleWidth)) * fx, 
                bottom = (camPosY - (Window::s_scaleHeight) / 2) * entity->scrollFactor.y, 
                top = (camPosY + Window::s_scaleHeight) * fy; 

    return (posX + width > left && 
            posX < right && 
            posY + height > bottom && 
            posY < top);
}


//-----------------------------


void Game::RenderEntities(std::vector<std::shared_ptr<Entity>> entities)
{
    for (auto& entity : entities)
        if ((entity.get() && entity))
        {
            #if STANDALONE == 0
                entity->renderable = CheckEntityRenderable(entity);
            #else
                if (entity->cull) 
                    entity->renderable = CheckEntityRenderable(entity);
            #endif

            if (entity->renderable) {
                entity->Render();
                Entity::s_rendered++;
            }  
        }
}


//-----------------------------


void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{
    const std::string ID = entity->ID;

    auto it = std::find(GetScene()->entities.begin(), GetScene()->entities.end(), entity);

    if (it != GetScene()->entities.end()) {
        it = GetScene()->entities.erase(std::move(it));
        --it;
    }

    else {
        auto UI_it = std::find(GetScene()->UI.begin(), GetScene()->UI.end(), entity);

        if (UI_it != GetScene()->UI.end())
        {
            UI_it = GetScene()->UI.erase(std::move(UI_it));
            --UI_it;
        }
    }

    entity->renderable = false;
    entity->active = false;
    entity->alive = false;

    if (entity->IsSprite())
    {
        const auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->GetBodies().size())
        {
            for (const auto &body : sprite->GetBodies())
                Physics::DestroyBody(body.first);

            sprite->GetBodies().clear();
        }
    }

    //remove from vector and disappear into the void
 
    if (entity.unique())
        entity.reset();

    //reset associated behavior if applicable

    auto behavior_it = std::find_if(GetScene()->behaviors.begin(), GetScene()->behaviors.end(), [&](auto b)
                                    { return b->ID == ID; });

    if (behavior_it != GetScene()->behaviors.end())
        (*behavior_it)->active = false;
}



/* entity containers for instantiation */

//----------------------------- sprite

std::shared_ptr<Sprite> Game::CreateSprite(const std::string& key, float x, float y, int frame, float scale, int layer, bool isSpawn)
{
    const auto sprite = std::make_shared<Sprite>(key, x, y, isSpawn);

    if (layer == 1)
        GetScene()->entities.emplace_back(sprite);

    if (layer == 2)
        GetScene()->UI.emplace_back(sprite);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData();
        sprite->SetFrame(frame);
    #endif

    sprite->SetScale(scale);

    return sprite;
}


//----------------------------- ui sprite


std::shared_ptr<Sprite> Game::CreateUI(const std::string& key, float x, float y, int frame)
{
    const Math::Vector2 pos = { x, y };

    const auto element = std::make_shared<Sprite>(key, pos);

    GetScene()->UI.emplace_back(element);

    #if STANDALONE == 1
        element->ReadSpritesheetData();
        element->SetFrame(frame);
    #endif

    return element;
}


//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{
    const auto ts = std::make_shared<Sprite>(key, x, y, false, true);

    GetScene()->entities.emplace_back(ts);

    ts->ReadSpritesheetData();
    ts->SetFrame(frame);

    return ts;
}


//----------------------------- text


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y, const std::string& font, int layer)
{
    const auto text = std::make_shared<Text>(content, x, y, font);

    if (layer == 1)
        GetScene()->entities.emplace_back(text);

    if (layer == 2)
        GetScene()->UI.emplace_back(text);

    return text;
}


//----------------------------- quad


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height, int layer, bool isStatic, bool isSpawn)
{
    const auto geom = std::make_shared<Geometry>(x, y, width, height, isSpawn);

    if (isStatic)
        geom->SetStatic(true);

    if (layer == 1)
        GetScene()->entities.emplace_back(geom);

    if (layer == 2)
        GetScene()->UI.emplace_back(geom);

    return geom;
}


//----------------------------- base entity wrapper


std::shared_ptr<Entity> Game::CreateEntity(int type, int layer)
{
    const auto entity = std::make_shared<Entity>(type);

    if (layer == 1)
        GetScene()->entities.emplace_back(entity);

    if (layer == 2)
        GetScene()->UI.emplace_back(entity);

    return entity;
}


//---------------------------- spawn


void Game::CreateSpawn(
    int type, 
    const std::string& filename, 
    float x, 
    float y, 
    float width, 
    float height, 
    float spawn_width, 
    float spawn_height, 
    const Math::Vector3& tint, 
    float alpha, 
    bool loop, 
    const std::string& behaviorName,
    const Scene::Spawn::Body& body
) 
{
    Scene::Spawn spawn;

    spawn.can_create = true; 
    spawn.type = type;
    spawn.filename = filename;
    spawn.posX = x;
    spawn.posY = y; 
    spawn.width = width;
    spawn.height = height;
    spawn.spawn_width = spawn_width;
    spawn.spawn_height = spawn_height;
    spawn.tint = tint;
    spawn.alpha = alpha;
    spawn.loop = loop;
    spawn.index = System::Utils::ReplaceFrom(spawn.filename, ".", "") + std::to_string(s_spawn_count);
    spawn.body = { 
        body.type, 
        body.shape, 
        body.exist, 
        body.is_sensor, 
        body.xOff, 
        body.yOff, 
        body.w, 
        body.h,
        body.density, 
        body.friction, 
        body.restitution
    };

    //append behaviors if name matches valid loaded behavior name

    if (behaviorName.length())
        spawn.behaviors_attached.emplace_back(behaviorName, false);

    GetScene()->spawns.emplace_back(spawn);

    s_spawn_count++;
}

