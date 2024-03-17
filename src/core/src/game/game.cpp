#include "../../../../build/include/app.h"
                                
  
//------------------------- base backend game layer functionality        


void Game::Boot()   
{   

    text->Init(); 

    time = new Time;
    camera = new Camera;
    physics = new Physics;
    maps = new MapManager;
    
	physics->world.SetContactListener(&physics->collisions);
    
    #if DEVELOPMENT == 1 
    
        physics->debug = new DebugDraw;

	    physics->world.SetDebugDraw(physics->debug);

    #endif 

    //preload / run game layer

    System::Application::game->Preload();
    System::Resources::Manager::RegisterAssets();
    System::Application::game->Run(System::Application::inputs, camera, physics);
    System::Application::inputs->CreateCursor();

    glfwSetWindowTitle(System::Window::s_instance, name.c_str());

    std::cout << "Game: " + name + " initialized.\n";

    gameState = true;

    
}


//-----------------------------


void Game::Exit() 
{

    gameState = false;

    entities.clear();
    System::Application::game->behaviors.clear();

    #if DEVELOPMENT == 1 
        delete physics->debug;
        physics->debug = nullptr;
    #endif

    delete camera;
    camera = nullptr;

    delete physics;
    physics = nullptr;

    delete time;
    time = nullptr;

    delete maps;
    maps = nullptr;

    std::cout << "Game: exited.\n";

}


//----------------------------


void Game::UpdateFrame()
{

    if (!gameState)
        return;

    #if DEVELOPMENT == 1 
        physics->debug->SetFlags(physics->m_flags);
    #endif

	physics->world.SetAllowSleeping(physics->sleeping);
	physics->world.SetWarmStarting(physics->setWarmStart);
	physics->world.SetContinuousPhysics(physics->continuous);
	physics->world.SetSubStepping(physics->subStep);
    physics->world.SetAutoClearForces(physics->clearForces);

    static double accumulator = 0.0;

    accumulator += time->m_delta;

    while (accumulator >= time->timeStep) {
        physics->world.Step(time->timeStep, physics->velocityIterations, physics->positionIterations);
        accumulator -= time->timeStep;
    }

    physics->Update();

    //render queues

    for (const auto &entity : entities)
        if ((entity.get() && entity) && entity.get()->m_renderable) {

            if (System::Application::game->cursor != nullptr)
                System::Application::game->cursor->SetDepth(entity->m_depth + 1);

            entity->Render();
        }
            
    //render input cursor

    System::Application::inputs->RenderCursor();

    //update behaviors

    for (const auto& behavior : System::Application::game->behaviors)
        if (behavior.get() && behavior)
            behavior->Update(System::Application::inputs, System::Application::game->camera);

    //depth sort

    std::sort(entities.begin(), entities.end(), [](auto a, auto b){ return a->m_depth < b->m_depth; });

    #if DEVELOPMENT == 1 

        if (physics->debug->enable) {
            physics->world.DebugDraw();
            physics->debug->Flush();
        }

    #endif

    //propagate input functionality to game instance
    
    System::Application::game->Update(System::Application::inputs, camera, physics);

}  


//---------------------------


void Game::DestroyUI()
{

    for (const auto &UI : entities)
    {
        std::vector<std::shared_ptr<Entity>>::iterator it = std::find(entities.begin() - 1, entities.end() - 1, UI);

        if (it != entities.end())
            entities.erase(it);

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

    std::vector<std::shared_ptr<Entity>>::iterator it = std::find(entities.begin() - 1, entities.end(), entity);

    if (it != entities.end())
       entities.erase(it);

    entity->m_renderable = false;

    entity->m_active = false;
    entity->m_alive = false;

    if (entity->IsSprite()) {

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) {
            for (const auto& body : sprite->bodies)
                physics->DestroyBody(body.first); 

            sprite->bodies.clear();
        }
    }

    entity.reset();
    entity = nullptr; 

} 



//-----------------------------


std::shared_ptr<Sprite> Game::CreateSprite(const std::string &key, float x, float y, int frame, float scale)
{

    auto sprite = std::make_shared<Sprite>(key, x, y, frame);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData(); 
    #endif

    sprite->SetScale(scale);

    entities.push_back(sprite);

    return sprite;
}



//-----------------------------


std::shared_ptr<Sprite> Game::CreateUI(const std::string &key, float x, float y, int frame)
{

    auto element = std::make_shared<Sprite>(key, x, y, "UI");

    #if STANDALONE == 1
        element->ReadSpritesheetData(); 
    #endif
    
    element->SetFrame(frame);

    entities.push_back(element);

    return element;
}


//-----------------------------


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string &key, float x, float y, int frame)
{

    auto ts = std::make_shared<Sprite>(key, x, y, frame, true);

    ts->type = "tile";
    //ts->m_shader = Shader::GetShader("sprite_batch");
    ts->ReadSpritesheetData(); 

    entities.push_back(ts);

    return ts;

}


//-----------------------------


std::shared_ptr<Text> Game::CreateText(const std::string &content, float x, float y)
{

    auto text = std::make_shared<Text>(content, x, y); 

    entities.push_back(text);

    return text;
}


//----------------------------- (rect)


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, float width, float height)
{
    auto geom = std::make_shared<Geometry>(x, y, width, height);

    entities.push_back(geom);

    return geom;
}


//----------------------------- (line)


std::shared_ptr<Geometry> Game::CreateGeom(float x, float y, const glm::vec2 &start, const glm::vec2 &end)
{
    auto geom = std::make_shared<Geometry>(x, y, start, end);

    entities.push_back(geom);

    return geom;
}