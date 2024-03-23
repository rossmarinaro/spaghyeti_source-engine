#include "../../../../build/include/app.h"
                                
  
//------------------------- base backend game layer functionality        


void Game::Boot()   
{   

    Game* game = System::Application::game;

    //game components

    game->text->Init(); 

    game->time = new Time;
    game->camera = new Camera;
    game->physics = new Physics;
    
    maps = new MapManager;
    
    //scene context handles

    game->context = { System::Application::inputs, game->camera, game->physics, game->time };

    //physics listener and debug
    
	game->physics->world.SetContactListener(&game->physics->collisions);
    
    #if DEVELOPMENT == 1 
    
        game->physics->debug = new DebugDraw;
	    game->physics->world.SetDebugDraw(game->physics->debug);

    #endif 

    //preload / run game layer

    game->Preload();

    System::Resources::Manager::RegisterAssets();

    game->Run(); 

    System::Application::inputs->CreateCursor();

    glfwSetWindowTitle(System::Window::s_instance, System::Application::name.c_str());

    std::cout << "Game: " + System::Application::name + " initialized.\n";

    gameState = true;

    
}


//-----------------------------


void Game::Exit() 
{

    gameState = false;

    Game* game = System::Application::game;

    game->entities.clear();

    behaviors.clear();

    #if DEVELOPMENT == 1 
        delete game->physics->debug;
        game->physics->debug = nullptr;
    #endif

    delete game->camera;
    game->camera = nullptr;

    delete game->physics;
    game->physics = nullptr;

    delete game->time;
    game->time = nullptr;

    delete maps;
    maps = nullptr;

    std::cout << "Game: exited.\n";

}


//----------------------------


void Game::UpdateFrame()
{

    if (!gameState)
        return;

    Game* game = System::Application::game;

    game->physics->Update();

    //render queues

    for (const auto& entity : game->entities)
        if ((entity.get() && entity) && entity.get()->m_renderable) {

            if (game->cursor != nullptr)
                game->cursor->SetDepth(entity->m_depth + 1);

            entity->Render();
        }
            
    //render input cursor

    System::Application::inputs->RenderCursor();

    //update behaviors, pass game process context to subclasses

    for (const auto& behavior : behaviors)
        if (behavior.get() && behavior)
            behavior->Update(game->context); 

    //depth sort

    std::sort(game->entities.begin(), game->entities.end(), [](auto a, auto b){ return a->m_depth < b->m_depth; });

    #if DEVELOPMENT == 1 

        if (game->physics->debug->enable) {
            game->physics->world.DebugDraw();
            game->physics->debug->Flush();
        }

    #endif

    //propagate input functionality to game instance
    
    game->Update();

}  


//---------------------------


void Game::DestroyUI()
{

    for (const auto& UI : System::Application::game->entities)
    {
        std::vector<std::shared_ptr<Entity>>::iterator it = std::find(System::Application::game->entities.begin() - 1, System::Application::game->entities.end() - 1, UI);

        if (it != System::Application::game->entities.end())
            System::Application::game->entities.erase(it);

        DestroyEntity(UI);
    }

}


//---------------------


void Game::RemoveFromVector(std::vector<std::shared_ptr<Sprite>>& vector, std::shared_ptr<Sprite> sprite)
{

    std::vector<std::shared_ptr<Sprite>>::iterator v_it = std::find(vector.begin() - 1, vector.end(), sprite);

    if (v_it != vector.end())
        vector.erase(v_it);

    std::vector<std::shared_ptr<Entity>>::iterator it = std::find(entities.begin() - 1, entities.end(), sprite);

    if (it != entities.end())
        entities.erase(it);

    DestroyEntity(sprite);

}


//----------------------------


void Game::RemoveFromVector(std::vector<std::shared_ptr<Text>>& vector, std::shared_ptr<Text> text)
{

    std::vector<std::shared_ptr<Text>>::iterator v_it = std::find(vector.begin() - 1, vector.end(), text);

    if (v_it != vector.end())
        vector.erase(v_it);

    std::vector<std::shared_ptr<Entity>>::iterator it = std::find(entities.begin() - 1, entities.end(), text);

    if (it != entities.end())
        entities.erase(it);

    DestroyEntity(text);

}


//-----------------------------

 
void Game::DestroyEntity(std::shared_ptr<Entity> entity)
{

    std::vector<std::shared_ptr<Entity>>::iterator it = std::find(System::Application::game->entities.begin() - 1, System::Application::game->entities.end(), entity);

    if (it != System::Application::game->entities.end())
       System::Application::game->entities.erase(it);

    entity->m_renderable = false;

    entity->m_active = false;
    entity->m_alive = false;

    if (entity->IsSprite()) {

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) {
            for (const auto& body : sprite->bodies)
                System::Application::game->physics->DestroyBody(body.first); 

            sprite->bodies.clear();
        }
    }

    entity.reset();
    entity = nullptr; 

} 



//-----------------------------


std::shared_ptr<Sprite> Game::CreateSprite(const std::string& key, float x, float y, int frame, float scale)
{

    auto sprite = std::make_shared<Sprite>(key, x, y, frame);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData(); 
    #endif

    sprite->SetScale(scale);

    System::Application::game->entities.push_back(sprite);

    return sprite;
}



//-----------------------------


std::shared_ptr<Sprite> Game::CreateUI(const std::string& key, float x, float y, int frame)
{

    auto element = std::make_shared<Sprite>(key, x, y, "UI");

    #if STANDALONE == 1
        element->ReadSpritesheetData(); 
    #endif
    
    element->SetFrame(frame);

    System::Application::game->entities.push_back(element);

    return element;
}


//-----------------------------


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{

    auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    ts->type = "tile";
    //ts->m_shader = Shader::GetShader("sprite_batch");
    ts->ReadSpritesheetData(); 

    System::Application::game->entities.push_back(ts);

    return ts;

}


//-----------------------------


std::shared_ptr<Text> Game::CreateText(const std::string& content, float x, float y)
{

    auto text = std::make_shared<Text>(content, x, y); 

    System::Application::game->entities.push_back(text);

    return text;
}


//----------------------------- (rect)


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height)
{
    auto geom = std::make_shared<Geometry>(x, y, width, height);

    System::Application::game->entities.push_back(geom);

    return geom;
}


//----------------------------- (line)


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, const glm::vec2 &start, const glm::vec2 &end)
{
    auto geom = std::make_shared<Geometry>(x, y, start, end);

    System::Application::game->entities.push_back(geom);

    return geom;
}