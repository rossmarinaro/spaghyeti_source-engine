#include "../app/app.h"
                                
  
//------------------------- base backend game layer functionality        


void Game::Boot()   
{   

    text->Init(); 

    time = new Time;
    camera = new Camera;
    physics = new Physics;

	physics->world.SetContactListener(&physics->collisions);
    
    #if DEVELOPMENT == 1 
    
        physics->debug = new DebugDraw;

	    physics->world.SetDebugDraw(physics->debug);

    #endif 

    //preload / run game layer

    System::Application::game->Preload();

    System::Resources::Manager::RegisterAssets();

    System::Application::game->Run(System::Application::game->camera);
    
    System::Application::inputs->CreateCursor();

    glfwSetWindowTitle(System::Window::s_instance, name.c_str());

    std::cout << "Game: " + name + " initialized.\n";

    gameState = true;

    
}


//-----------------------------


void Game::Exit() 
{

    gameState = false;

    spriteQueue.clear();
    debugQueue.clear();

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

    std::cout << "Game: exited.\n";

}


//----------------------------


void Game::UpdateFrame()
{

    if (!gameState)
        return;

    physics->CleanupRemovedBodies();

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

    while (accumulator >= time->timeStep)
    {
        physics->world.Step(time->timeStep, physics->velocityIterations, physics->positionIterations);
        accumulator -= time->timeStep;
    }

    //render queues
 
    for (const auto &sprite : spriteQueue)
        if (sprite.get() && sprite.get()->m_renderable)
            sprite->Render(); 

    for (const auto &UI : UIQueue)
        if ((UI.get() && UI) && UI.get()->m_renderable)
            UI->Render();

    for (const auto &text : textQueue)
        if ((text.get() && text) && text.get()->m_renderable)
            text->Render();

    for (const auto &graphic : debugQueue)
        if ((graphic.get() && graphic) && graphic.get()->m_debug)
            graphic->Render();


    //render input cursor

    System::Application::inputs->RenderCursor();

    #if DEVELOPMENT == 1 

        if (physics->debug->enable) {
            physics->world.DebugDraw();
            physics->debug->Flush();
        }

    #endif

    //depth sort

    std::sort(spriteQueue.begin(), spriteQueue.end(), [](auto a, auto b){ return a->m_depth > b->m_depth; });
    std::sort(textQueue.begin(), textQueue.end(), [](auto a, auto b){ return a->m_depth > b->m_depth; });
    std::sort(UIQueue.begin(), UIQueue.end(), [](auto a, auto b){ return a->m_depth > b->m_depth; });


    //propagate input functionality to game instance
    
    System::Application::game->Update(System::Application::inputs, System::Application::game->camera);

}  


//---------------------------


void Game::DestroyUI()
{

    for (const auto &UI : UIQueue)
    {
        std::vector<std::shared_ptr<Sprite>>::iterator it = std::find(UIQueue.begin() - 1, UIQueue.end() - 1, UI);

        if (it != UIQueue.end())
            UIQueue.erase(it);

        DestroySprite(UI);
    }

}


//---------------------


void Game::RemoveFromVector(std::vector<std::shared_ptr<Sprite>>& vector, std::shared_ptr<Sprite> sprite)
{

    std::vector<std::shared_ptr<Sprite>>::iterator it = std::find(vector.begin() - 1, vector.end() - 1, sprite);

    if (it != vector.end())
        vector.erase(it);

    DestroySprite(sprite);

}


//----------------------------


void Game::RemoveFromVector(std::vector<std::shared_ptr<Text>>& vector, std::shared_ptr<Text> text)
{

    std::vector<std::shared_ptr<Text>>::iterator it = std::find(vector.begin() - 1, vector.end() - 1, text);

    if (it != vector.end())
        vector.erase(it);

    DestroyText(text);

}


//-----------------------------

 
void Game::DestroySprite(std::shared_ptr<Sprite> sprite)
{

    std::vector<std::shared_ptr<Sprite>>::iterator it = std::find(spriteQueue.begin() - 1, spriteQueue.end() - 1, sprite);

    if (it != spriteQueue.end())
       spriteQueue.erase(it);
    
    sprite->m_renderable = false;

    sprite->m_active = false;
    sprite->m_alive = false;

    if (sprite->m_body.self != nullptr)
       physics->bodiesToRemove.insert(sprite);

    sprite.reset();
    sprite = nullptr; 

}

//-----------------------------

 
void Game::DestroyText(std::shared_ptr<Text> text)
{

    std::vector<std::shared_ptr<Text>>::iterator it = std::find(textQueue.begin() - 1, textQueue.end() - 1, text);

    if (it != textQueue.end())
        textQueue.erase(it);

    text.reset();
    text = nullptr; 

}


//-----------------------------

 
void Game::DestroyGraphic(std::shared_ptr<Graphics::Rectangle> graphic)
{

    std::vector<std::shared_ptr<Graphics::Shape>>::iterator g_it = std::find(debugQueue.begin() - 1, debugQueue.end() - 1, graphic);

    if (g_it != debugQueue.end())
        debugQueue.erase(g_it);

    graphic->m_debug = false;

    graphic.reset();
    graphic = nullptr;
    

}


//-----------------------------


std::shared_ptr<Sprite> Game::CreateSprite(const std::string &key, float x, float y, int frame, float scale)
{

    auto sprite = std::make_shared<Sprite>(key, glm::vec2(x, y), frame);

    sprite->SetScale(scale);

    spriteQueue.push_back(sprite);

    entities.insert(sprite);

    return sprite;
}


//-----------------------------


std::shared_ptr<Sprite> Game::CreateUI(const std::string &key, float x, float y, int frame)
{

    auto element = std::make_shared<Sprite>(key, glm::vec2(x, y), "UI");
    
    element->SetFrame(frame);

    UIQueue.push_back(element);

    entities.insert(element);

    return element;
}



//-----------------------------


std::shared_ptr<Text> Game::CreateText(const std::string &content, float x, float y)
{

    auto text = std::make_shared<Text>(content, x, y); 

    textQueue.push_back(text);

    return text;
}


//-----------------------------


std::shared_ptr<Graphics::Rectangle> Game::CreateRect(float x, float y, float width, float height)
{

    auto rect = std::make_shared<Graphics::Rectangle>(x, y, width, height);

    debugQueue.push_back(rect);

    return rect;
}


//-----------------------------


std::shared_ptr<Graphics::Rectangle> Game::CreateRect(auto &sprite)
{

    auto rect = std::make_shared<Graphics::Rectangle>(sprite);

    debugQueue.push_back(rect);

    return rect;
}

