#include "../../../build/sdk/include/app.h"

#if DEVELOPMENT == 1 && STANDALONE == 1

    #include "../../../build/sdk/include/displayInfo.h"
    DisplayInfo* displayInfo;

#endif

using namespace System;      


//-----------------------

void FlushGame(Game* game)
{

    game->inputs->ResetControls();

    if (Application::eventPool) {
        delete Application::eventPool; 
        Application::eventPool = nullptr;
    }

    for (auto& entity : game->currentScene->entities) 
        entity.reset();

    for (auto& ui : game->currentScene->UI) 
        ui.reset();

    for (auto& behavior : game->currentScene->behaviors) 
        behavior.reset();

    game->currentScene->entities.clear();
    game->currentScene->UI.clear();
    game->currentScene->behaviors.clear();

    game->maps->layers.clear();

    //refresh physics
    
    game->physics->ClearBodies();   

    #if DEVELOPMENT == 1 

       delete game->physics->debug;
       game->physics->debug = nullptr;

        #if STANDALONE == 1

            delete displayInfo;
            displayInfo = nullptr;

        #endif

        game->physics->debug = new DebugDraw;
	    game->physics->GetWorld().SetDebugDraw(game->physics->debug);

        #if STANDALONE == 1
            displayInfo = new DisplayInfo;
        #endif

    #endif  

    delete game->physics;
    game->physics = nullptr;

    game->physics = new Physics;

    game->physics->GetWorld().SetContactListener(&game->physics->collisions);

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

    Game* game = Application::game;

    game->currentScene = nullptr;

    game->text->Init(); 

    //preload / run game layer

    StartScene(game->scenes[0]->key); 

    glfwSetWindowTitle(Window::s_instance, Application::name.c_str());

    game->inputs->CreateCursor();

    game->physics->GetWorld().SetContactListener(&game->physics->collisions);
    
    //physics listener and debug

    #if DEVELOPMENT == 1 
     
        game->physics->debug = new DebugDraw;
	    game->physics->GetWorld().SetDebugDraw(game->physics->debug);

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

        if (game->currentScene) {
            Resources::Manager::Clear(false);  
            FlushGame(game);  
        }   

        Application::eventPool = new EventPool(THREAD_COUNT);

        //assign / load current scene

        game->currentScene = *it;

        game->currentScene->Preload();

        game->currentScene->vignette = std::make_unique<Sprite>("base", 0.0f, -50.0f);
        game->currentScene->vignette->SetTint({ 0.0f, 0.0f, 0.0f });
        game->currentScene->vignette->SetAlpha(0.0f);

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

    Game* game = Application::game;

    game->m_gameState = false;

    FlushGame(game);

    for (auto& scene : game->scenes) {
        delete scene;
        scene = nullptr;
    }

    game->scenes.clear();    
    game->text->ShutDown();

    #if DEVELOPMENT == 1 

       delete game->physics->debug;
       game->physics->debug = nullptr;

        #if STANDALONE == 1

            delete displayInfo;
            displayInfo = nullptr;

        #endif

    #endif

    delete game->camera;
    game->camera = nullptr;

    delete game->physics;
    game->physics = nullptr;

    delete game->time;
    game->time = nullptr;

    delete game->inputs;
    game->inputs = nullptr;

    delete maps;
    maps = nullptr;

    std::cout << "Game: exited.\n";

}


//----------------------------


void Game::UpdateFrame()
{

    Game* game = Application::game;

    if (!game->m_gameState)
        return;

    game->physics->Update();

    //render queues

    for (const auto& entity : game->currentScene->entities)
        if ((entity.get() && entity) && entity.get()->renderable) 
            entity->Render();

    for (const auto& UI : game->currentScene->UI)
        if ((UI.get() && UI) && UI.get()->renderable) 
            UI->Render();

    //vignette overlay

    game->currentScene->vignette->texture.FrameWidth = Window::s_scaleWidth * 4;
    game->currentScene->vignette->texture.FrameHeight = Window::s_scaleHeight * 4;
    game->currentScene->vignette->Render();

    //render input cursor

    Application::game->inputs->RenderCursor();
 
    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : game->currentScene->behaviors) 
    {

        if (!behavior.get() || !behavior)
            continue;

        if ((!game->currentScene->IsPaused() && behavior->layer == 0)) 
            behavior->Update();  
            
        else if (behavior->layer == 1)
            behavior->Update();
    }

    //depth sort

    std::sort(game->currentScene->entities.begin(), game->currentScene->entities.end(), [](auto a, auto b){ return a->depth < b->depth; });

    #if DEVELOPMENT == 1 

        if (game->physics->debug->enable) 
        {
            game->physics->GetWorld().DebugDraw();
            game->physics->debug->Flush();

            #if STANDALONE == 1
                displayInfo->Update(game->m_context);
            #endif
        }

    #endif

}  


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

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

    if (!Application::game->currentScene->entities.size())
       entity.reset();

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



