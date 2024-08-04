#include "../../../build/sdk/include/app.h"

#if DEVELOPMENT == 1 && STANDALONE == 1

    #include "../../../build/sdk/include/displayInfo.h"
    DisplayInfo* displayInfo;

#endif

using namespace System;      


//-----------------------

void Game::Flush()
{

    inputs->ResetControls();

    if (Application::eventPool) {
        delete Application::eventPool; 
        Application::eventPool = nullptr;
    }

    // for (auto& entity : currentScene->entities) 
    //     entity.reset();

    // for (auto& ui : currentScene->UI) 
    //     ui.reset();

    // for (auto& behavior : game->currentScene->behaviors) 
    //     behavior.reset();

    currentScene->entities.clear();
    currentScene->UI.clear();
    currentScene->behaviors.clear();

    maps->layers.clear();

    //refresh physics
    
    physics->ClearBodies();   

    #if DEVELOPMENT == 1 

       delete physics->debug;
       physics->debug = nullptr;

        #if STANDALONE == 1

            delete displayInfo;
            displayInfo = nullptr;

        #endif

        physics->debug = new DebugDraw;
	    physics->GetWorld().SetDebugDraw(physics->debug);

        #if STANDALONE == 1
            displayInfo = new DisplayInfo;
        #endif

    #endif  

    delete physics;
    physics = nullptr;

    physics = new Physics;

    physics->GetWorld().SetContactListener(&physics->collisions);

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

    this->inputs = new Inputs;
    this->time = new Time;
    this->camera = new Camera;
    this->physics = new Physics;

    maps = new MapManager;

    //scene context handles

    this->m_context = { this->inputs, this->camera, this->physics, this->time };

}


//---------------------------- boot entry scene


void Game::Boot()   
{     

    std::cout << "Game: " + Application::name + " initializing.\n";

    currentScene = nullptr;

    text->Init(); 

    //preload / run game layer

    StartScene(scenes[0]->key); 

    glfwSetWindowTitle(Window::s_instance, Application::name.c_str());

    inputs->CreateCursor();

    physics->GetWorld().SetContactListener(&physics->collisions);
    
    //physics listener and debug

    #if DEVELOPMENT == 1 
     
        physics->debug = new DebugDraw;
	    physics->GetWorld().SetDebugDraw(physics->debug);

        #if STANDALONE == 1
            displayInfo = new DisplayInfo;
        #endif

    #endif

}


//-----------------------------


void Game::StartScene(const std::string& key) 
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
            if (game->currentScene->key != key)
                Resources::Manager::Clear(false);  

            game->Flush();
        }   

        Application::eventPool = new EventPool(THREAD_COUNT);

        //assign / load current scene

        game->currentScene = *it;

        if (std::find(cachedScenes.begin(), cachedScenes.end(), game->currentScene->key) == cachedScenes.end()) {
            game->currentScene->Preload();
            cachedScenes.push_back(game->currentScene->key);
        }

        game->currentScene->vignette = std::make_unique<Sprite>("base", 0.0f, -50.0f);
        game->currentScene->vignette->SetTint({ 0.0f, 0.0f, 0.0f });
        game->currentScene->vignette->SetAlpha(0.0f);
        game->currentScene->vignette->shader = Shader::GetShader("UI");
        game->currentScene->vignette->SetScrollFactor({ 0.0f, 1.0f });

        game->currentScene->Run();  

        game->m_gameState = true; 

        #if DEVELOPMENT == 1
            std::cout << "Scene: " + key + " started.\n";
        #endif
    }

    else {
        #if DEVELOPMENT == 1
            std::cout << "Scene: key not found.\n";
        #endif
    }

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

    std::cout << "Game: exited.\n";

}


//----------------------------


void Game::UpdateFrame()
{

    if (!m_gameState)
        return;

    physics->Update();

    //render queues

    for (const auto& entity : currentScene->entities)
        if ((entity.get() && entity) && entity.get()->renderable) 
            entity->Render();

    for (const auto& UI : currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render();

    //vignette overlay

    currentScene->vignette->texture.FrameWidth = Window::s_scaleWidth * 4;
    currentScene->vignette->texture.FrameHeight = Window::s_scaleHeight * 4;
    currentScene->vignette->Render();

    //render input cursor

    Application::game->inputs->RenderCursor();
 
    //update behaviors, pass game process context to subclasses

    auto inactive_behavior_it = std::find_if(currentScene->behaviors.begin(), currentScene->behaviors.end(), [](auto b) { return b->active == false; });

    if (inactive_behavior_it != currentScene->behaviors.end()) {
        //Application::game->currentScene->behaviors.erase(inactive_behavior_it);
        //(*inactive_behavior_it).reset();
    }

    for (const auto& behavior : currentScene->behaviors) 
        if (behavior->active) 
        {

            if ((!currentScene->IsPaused() && behavior->layer == 0)) 
                behavior->Update();  
                
            else if (behavior->layer == 1)
                behavior->Update();
        }


    //depth sort

    std::sort(currentScene->entities.begin(), currentScene->entities.end(), [](auto a, auto b){ return a->depth < b->depth; });

    #if DEVELOPMENT == 1 

        if (physics->debug->enable) 
        {
            physics->GetWorld().DebugDraw();
            physics->debug->Flush();

            #if STANDALONE == 1
                displayInfo->Update(m_context);
            #endif
        }

    #endif

}  


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

    const std::string ID = entity->ID;

    auto it = std::find(Application::game->currentScene->entities.begin(), Application::game->currentScene->entities.end(), entity);

    if (it != Application::game->currentScene->entities.end())
        Application::game->currentScene->entities.erase(it);
    
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


std::shared_ptr<Sprite> Game::CreateSprite(const std::string& key, float x, float y, int frame, float scale)
{

    auto sprite = std::make_shared<Sprite>(key, x, y, frame);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData(); 
    #endif

    sprite->SetScale(scale);

    Application::game->currentScene->entities.push_back(sprite); 

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

    Application::game->currentScene->UI.push_back(element);

    return element;
}


//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{

    auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    ts->type = "tile";
    //ts->shader = Shader::GetShader("batch");
    ts->ReadSpritesheetData(); 

    Application::game->currentScene->entities.push_back(ts);

    return ts;

}


//----------------------------- debug text


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y)
{

    auto text = std::make_shared<Text>(content, x, y); 

    Application::game->currentScene->UI.push_back(text);

    return text;
}


//----------------------------- quad


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height)
{
    auto geom = std::make_shared<Geometry>(x, y, width, height);

    Application::game->currentScene->UI.push_back(geom);

    return geom;
}


//----------------------------- base entity wrapper


std::shared_ptr<Entity> Game::CreateEntity(const std::string& type)
{
    auto entity = std::make_shared<Entity>(type.c_str());

    Application::game->currentScene->entities.push_back(entity);

    return entity;
} 



