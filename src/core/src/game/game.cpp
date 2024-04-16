#include "../../../../build/sdk/include/app.h"

#if DEVELOPMENT == 1 && STANDALONE == 1

    #include "../../../../build/sdk/include/displayInfo.h"
    DisplayInfo* displayInfo;

#endif

using namespace System;                           
  
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

    this->context = { this->inputs, this->camera, this->physics, this->time };

}


//---------------------------- boot entry scene


void Game::Boot()   
{   

    Game* game = Application::game;

    game->text->Init(); 

    //preload / run game layer

    StartScene(game->scenes[0]->key); 

    glfwSetWindowTitle(Window::s_instance, Application::name.c_str());

    game->inputs->CreateCursor();

    game->physics->world.SetContactListener(&game->physics->collisions);
    
    //physics listener and debug

    #if DEVELOPMENT == 1 
    
        game->physics->debug = new DebugDraw;
	    game->physics->world.SetDebugDraw(game->physics->debug);

        #if STANDALONE == 1
            displayInfo = new DisplayInfo;
        #endif

    #endif

    std::cout << "Game: " + Application::name + " initialized.\n";
    
}


//-----------------------------


void Game::StartScene(const std::string& key) 
{

    Game* game = Application::game;

    game->gameState = false;
    game->time->exitFlag = true;

    //find loaded scene

    auto it = std::find_if(game->scenes.begin(), game->scenes.end(), [&](std::shared_ptr<Scene> scene) { return scene->key == key; });

    if (it != game->scenes.end())
    {

        //clear entities if applicable

        if (game->currentScene) {
            game->currentScene->entities.clear();
            game->currentScene->behaviors.clear();
        }

        //assign / load current scene

        game->currentScene = *it; 

        Resources::Manager::Clear(false);

        game->currentScene->Preload();
        game->currentScene->Run();
        
        game->time->exitFlag = false; 
        game->gameState = true;

    }
}


//-----------------------------


void Game::Exit() 
{

    Game* game = Application::game;

    game->gameState = false;
    game->time->exitFlag = game->time->exitFlag.exchange(1);

    game->currentScene->entities.clear();
    game->currentScene->behaviors.clear();

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

    if (!game->gameState)
        return;

    game->physics->Update();

    //render queues

    for (const auto& entity : game->currentScene->entities)
        if ((entity.get() && entity) && entity.get()->m_renderable) {

            if (game->cursor != nullptr)
                game->cursor->SetDepth(entity->m_depth + 1);

            entity->Render();
        }

    //render input cursor

    Application::game->inputs->RenderCursor();

    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : game->currentScene->behaviors)
        if (!game->currentScene->IsPaused() && behavior.get() && behavior)
            behavior->Update(game->context, game->currentScene->behaviors); 

    //depth sort

    std::sort(game->currentScene->entities.begin(), game->currentScene->entities.end(), [](auto a, auto b){ return a->m_depth < b->m_depth; });

    #if DEVELOPMENT == 1 

        if (game->physics->debug->enable) {
            game->physics->world.DebugDraw();
            game->physics->debug->Flush();
        }

        #if STANDALONE == 1
            displayInfo->Update(game->context);
        #endif

    #endif

    //propagate input functionality to game instance
    
    game->currentScene->Update();

}  


//---------------------------


void Game::DestroyUI()
{

    for (const auto& UI : Application::game->currentScene->entities)
    {
        std::vector<std::shared_ptr<Entity>>::iterator it = std::find(Application::game->currentScene->entities.begin() - 1, Application::game->currentScene->entities.end() - 1, UI);

        if (it != Application::game->currentScene->entities.end())
            Application::game->currentScene->entities.erase(it);

        DestroyEntity(UI);
    }

}


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

    std::vector<std::shared_ptr<Entity>>::iterator it = std::find(Application::game->currentScene->entities.begin() - 1, Application::game->currentScene->entities.end(), entity);

    if (it != Application::game->currentScene->entities.end())
       Application::game->currentScene->entities.erase(it);

    entity->m_renderable = false;

    entity->m_active = false;
    entity->m_alive = false;

    if (entity->IsSprite()) {

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) {
            for (const auto& body : sprite->bodies)
                Physics::DestroyBody(body.first); 

            sprite->bodies.clear();
        }
    }

    entity.reset();
    entity = nullptr; 

} 



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

    auto element = std::make_shared<Sprite>(key, x, y, "UI");

    #if STANDALONE == 1
        element->ReadSpritesheetData(); 
    #endif
    
    element->SetFrame(frame);

    Application::game->currentScene->entities.push_back(element);

    return element;
}


//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{

    auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    ts->type = "tile";
    //ts->m_shader = Shader::GetShader("sprite_batch");
    ts->ReadSpritesheetData(); 

    Application::game->currentScene->entities.push_back(ts);

    return ts;

}


//----------------------------- debug text


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y)
{

    auto text = std::make_shared<Text>(content, x, y); 

    Application::game->currentScene->entities.push_back(text);

    return text;
}


//----------------------------- (quad)


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height)
{
    auto geom = std::make_shared<Geometry>(x, y, width, height);

    Application::game->currentScene->entities.push_back(geom);

    return geom;
}
