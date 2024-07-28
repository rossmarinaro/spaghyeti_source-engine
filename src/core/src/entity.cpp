#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/manager.h"


//---------------------------------- empty entity

Entity::Entity(const char* type) {
    this->type = type;
    name = "Untitled_" + std::to_string(s_count);
    s_count++;
}


//------------------------------------ active entity 


Entity::Entity(const char* type, float x, float y):
    m_model(glm::mat4(1.0f)),
    m_scrollFactor(glm::vec2(1.0f))
{ 
    this->type = type;
    position = glm::vec2(x, y);
    scale = glm::vec2(1.0f); 
    rotation = 0.0f;  
    alpha = 1.0f;
    tint = glm::vec3(1.0f, 1.0f, 1.0f);
    active = true;
    alive = true;
    renderable = true;
    flipX = false;
    flipY = false;
    depth = s_depth + 1;

    name = "Untitled_" + std::to_string(s_count);
    s_count++; 
}


//------------------------------------


void Entity::StartFollow(Camera* camera, float offset) 
{

    camera->targetX = position.x;
    camera->targetY = position.y;

    if (camera->InBounds())
        camera->position.x = (-position.x + offset) / 2;  
}


//------------------------------------


void Entity::SetData(const std::string& key, const std::any& value) 
{ 

    auto it = data.find(key);

    if (it != data.end())
        data.erase(it);

    data.insert({ key, value }); 
}


//------------------------------------ 


void Entity::Cull(std::shared_ptr<Entity> target, float factor, int distance)
{

    auto context = System::Game::GetScene()->GetContext();

    if (!context.camera->InBounds()) {
        SetScrollFactor({ 1.0f, 1.0f });
        renderable = (position.x > target->position.x && position.x < target->position.x + 1500) ||
                           (position.x < target->position.x && position.x > target->position.x - System::Window::s_scaleWidth);
    }

    else 
        renderable = (position.x > target->position.x && position.x < target->position.x + distance) ||
                           (position.x < target->position.x && position.x > target->position.x - System::Window::s_scaleWidth * factor);

}


/* GEOMETRY */


//quad
Geometry::Geometry(float x, float y, float width, float height): 
    Entity("geometry", x, y),
        m_type("quad")
{ 
    this->width = width;
    this->height = height;
    tint = glm::vec3(0.0f, 0.0f, 1.0f);
    texture = Graphics::Texture2D::GetTexture("base");
    shader = Shader::GetShader("graphics");
    
    #if DEVELOPMENT == 1
        std::cout << "Entity: quad created.\n"; 
    #endif
}


//------------------------------------- 


void Geometry::Render()
{

    m_model = glm::mat4(1.0f); 

    auto SetShader = [&](){
        shader.SetVec3f("tint", tint, true);
        shader.SetMat4("model", m_model, true);  
        shader.SetFloat("alphaVal", alpha, true);
    };

    //quad or line

    if (strcmp(m_type, "quad") == 0)
    {

        texture.FrameWidth = width;
        texture.FrameHeight = height;

        m_model = glm::translate(m_model, glm::vec3(position, 0.0f));  

        m_model = glm::translate(m_model, glm::vec3(0.5f * width + position.x, 0.5f * height + position.y, 0.0f)); 

        if (rotation > 0)
            m_model = glm::rotate(m_model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

        m_model = glm::translate(m_model, glm::vec3(-0.5f * width - position.x, -0.5f * height - position.y, 0.0f));
        
        SetShader();

        texture.Update(position, false, false, m_drawStyle); 

    }


}


/* SPRITE */


std::shared_ptr<Sprite> Sprite::Clone() 
{
    auto clone = std::make_shared<Sprite>(*this);

    clone->ReadSpritesheetData();

    System::Application::game->currentScene->entities.push_back(clone); 

    if (bodies.size())
        for (int i = 0; i < bodies.size(); i++)
            clone->bodies.push_back({ 
                Physics::CreateDynamicBody("box", 
                    bodies[0].second.x, 
                    bodies[0].second.y, 
                    bodies[0].second.z, 
                    bodies[0].second.w, 
                    bodies[0].first->GetFixtureList()->IsSensor(), 
                    bodies[0].first->GetFixtureList()->GetUserData().pointer, 
                    bodies[0].first->GetFixtureList()->GetDensity(), 
                    bodies[0].first->GetFixtureList()->GetFriction(), 
                    bodies[0].first->GetFixtureList()->GetRestitution()), { 
                        bodies[0].second.z, 
                        bodies[0].second.w, 
                        bodies[0].second.z, 
                        bodies[0].second.w 
                    } 
                });
      
    return clone;
}


//------------------------------------ velocity, use physics if available / else default to position update


void Sprite::SetVelocity(float velX, float velY) 
{ 

    if (!active)
        return;

    velocityX = velX;
    velocityY = velY;

    if (bodies.size()) 
        bodies[0].first->SetLinearVelocity(b2Vec2(velocityX, velocityY));

    else {
        position.x += velocityX /* * System::Application::game->time->GetSeconds() */; 
        position.y += velocityY /* * System::Application::game->time->GetSeconds() */; 
    }

}


//--------------------------------


void Sprite::SetVelocityX(float velX) 
{ 

    if (!active)
        return;

    velocityX = velX; 

    if (bodies.size() && IsContacting()) 
        bodies[0].first->SetLinearVelocity(b2Vec2(velocityX, bodies[0].first->GetLinearVelocity().y));

    else
        position.x += velocityX; // System::Application::game->time->GetSeconds();     
}


//---------------------------------


void Sprite::SetVelocityY(float velY) 
{ 

    if (!active)
        return;

    velocityY = velY;

    if (bodies.size()) 
        bodies[0].first->SetLinearVelocity(b2Vec2(bodies[0].first->GetLinearVelocity().x, velocityY));
    
    else
        position.y += velocityY; // System::Application::game->time->GetSeconds(); 
}


//----------------------------- set impulse x


void Sprite::SetImpulse(float x, float y) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(b2Vec2(x * 10000, y * 10000), bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- set impulse x


void Sprite::SetImpulseX(float x) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(b2Vec2(x * 10000, bodies[0].first->GetLinearVelocity().y), bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- set impulse y


void Sprite::SetImpulseY(float y) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(b2Vec2(bodies[0].first->GetLinearVelocity().x, y * 10000), bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- remove bodies


void Sprite::RemoveBodies() 
{

    //reset texture position to normal coords

    float x = bodies[0].first->GetPosition().x / 2,
          y = bodies[0].first->GetPosition().y / 2;

    for (auto it = bodies.begin(); it != bodies.end(); ++it) 
        Physics::DestroyBody((*it).first);

    bodies.clear();

    SetPosition(x, y);
}


//----------------------------- set texture


void Sprite::SetTexture(const std::string& key)
{  

    auto texture = Graphics::Texture2D::GetTexture(key); 
        
    this->key = key; 

    texture.FrameWidth = texture.Width;
    texture.FrameHeight = texture.Height;
    texture.ID = texture.ID;

    m_currentAnim = {};
}


//----------------------------- read frame data


void Sprite::ReadSpritesheetData()
{    
    
    const char* spritesheet = System::Resources::Manager::GetSpritesheetPath(key);
 
    //json file

    if (System::Utils::str_endsWith(spritesheet, ".json")) 
    {

        #if USE_JSON == 1 

            std::ifstream JSON(spritesheet);

            json data = json::parse(JSON);

                for (const auto& frame : data["frames"])
                    if (frame.contains("frame")) 
                    {
                        int x = frame["frame"]["x"],
                            y = frame["frame"]["y"],
                            w = frame["frame"]["w"],
                            h = frame["frame"]["h"];

                        m_resourceData.push_back({ x, y, w, h });
                    }
        #else 

            #if DEVELOPMENT == 1
                std::cout << "Sprite: an attempt to load JSON failed, because JSON has not been enabled.\n";
            #endif
            
        #endif

    }

    //int array

    else if (System::Resources::Manager::GetRawSpritesheetData(key).size())
        m_resourceData = System::Resources::Manager::GetRawSpritesheetData(key);  

    //not a spritesheet

    else 
        return;
    
    m_isSpritesheet = true;
    frames = m_resourceData.size();

}



//---------------------- animate sprite


void Sprite::Animate(const std::string& animKey, bool yoyo, int rate)
{ 

    uint32_t seconds = System::Application::game->time->GetSeconds() * rate;

    m_currentAnim = { animKey, { yoyo, rate } }; 

    int startFrame = anims.find(m_currentAnim.first)->second.first,
        endFrame = anims.find(m_currentAnim.first)->second.second,
        frame = yoyo ? startFrame : endFrame;

    m_animComplete = frame == currentFrame;

    try {

        if (m_isSpritesheet && active)
        {

            std::map<std::string, std::pair<int, int>>::iterator anim = anims.find(animKey);

            if (anim == anims.end() || System::Application::game->currentScene->IsPaused()) 
                return;

            std::vector<int> frames; //frames to populate  

            if (yoyo)
            {

                for (int i = anim->second.first; i < anim->second.second + 1; i++) 
                    frames.push_back(i);
                
                uint32_t elapsed = seconds % frames.size();

                std::vector<int> frames_reversed;

                for (int i = anim->second.second; i > anim->second.first - 1; i--) 
                    frames_reversed.push_back(i);

                uint32_t elapsed_reversed = seconds % frames_reversed.size();

                if (!m_anim_yoyo && currentFrame == anim->second.second && frames[elapsed] != anim->second.second) 
                    m_anim_yoyo = true;

                if (m_anim_yoyo && currentFrame == anim->second.first && frames_reversed[elapsed_reversed] != anim->second.first)    
                    m_anim_yoyo = false;

                if (m_anim_yoyo) 
                    SetFrame(frames_reversed[elapsed_reversed]);

                else 
                    SetFrame(frames[elapsed]);
             
            }

            else    
            {
                for (int i = anim->second.first; i < anim->second.second; i++) 
                    frames.push_back(i);
                
                uint32_t elapsed = seconds % frames.size();

                SetFrame(currentFrame != anim->second.second ? frames[elapsed] : anim->second.first);
            }

        }
    }

    catch (std::runtime_error& err) { 
        #if DEVELOPMENT == 1
            std::cout << "Sprite: error playing animation: " << err.what() << "\n"; 
        #endif
    }
}


//------------------------------------------ render sprite / update transformations


void Sprite::Render()
{  

    if (!renderable || !alive)
        return;

    //update spritesheet UV subtexturing if applicable

    if (m_isSpritesheet) 
    {
        
        float currentFrameX = (float)m_resourceData[currentFrame][0],
              currentFrameY = (float)m_resourceData[currentFrame][1],
              currentFrameWidth = (float)m_resourceData[currentFrame][2],
              currentFrameHeight = (float)m_resourceData[currentFrame][3],
              factorX = (float)m_resourceData[currentFrame][4],
              factorY = (float)m_resourceData[currentFrame][5];

        texture.FrameWidth = currentFrameWidth;
        texture.FrameHeight = currentFrameHeight;

        //tilemap

        if (strcmp(type, "tile") == 0)
        {
            texture.U1 = (currentFrameX * currentFrameWidth) / texture.Width;      
            texture.U2 = ((currentFrameX + 1) * currentFrameWidth) / texture.Width;

            texture.V1 = (currentFrameY * currentFrameHeight) / texture.Height; 
            texture.V2 = ((currentFrameY + 1) * currentFrameHeight) / texture.Height; 
        }

        //generic

        else 
        {
            texture.U1 = (currentFrameX * factorX) / texture.Width;      
            texture.U2 = ((currentFrameX + currentFrameWidth) * factorX) / texture.Width;

            texture.V1 = (currentFrameY * factorY) / texture.Height; 
            texture.V2 = ((currentFrameY + currentFrameHeight) * factorY) / texture.Height; 
        }

    }

    //sprite model transformations

    m_model = glm::mat4(1.0f);       

    m_model = glm::translate(m_model, { 0.5f * texture.FrameWidth + position.x, 0.5f * texture.FrameHeight + position.y, 0.0f }); 
    
    //apply scaling to sprites that have bodies
    
    if (bodies.size())
        m_model = glm::scale(m_model, glm::vec3(scale, 1.0f));   

    m_model = glm::rotate(m_model, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); 
    m_model = glm::translate(m_model, { -0.5f * texture.FrameWidth - position.x, -0.5f * texture.FrameHeight - position.y, 0.0f });

    //update shaders and textures 

   // if (strcmp(type, "tile") == 0)
        //shader.SetInt("images", 1, true);
    //else
        shader.SetInt("image", 0, true);

    #ifndef __EMSCRIPTEN__
        shader.SetInt("repeat", texture.Repeat, true);
    #endif

    shader.SetVec2f("scale", glm::vec2(
        bodies.size() ? 1.0f : scale.x, 
        bodies.size() ? 1.0f : scale.y
    ), true);
    
    shader.SetFloat("alphaVal", alpha, true); 
    shader.SetVec3f("tint", tint, true);
    shader.SetMat4("model", m_model, true);

    if (IsSprite())
        shader.SetMat4("view", glm::translate(glm::mat4(1.0f), glm::vec3(System::Application::game->camera->position.x * m_scrollFactor.x, System::Application::game->camera->position.y * m_scrollFactor.y, 0.0f)), true);
    
    else         
        shader.SetMat4("view", glm::mat4(1.0f), true);

    #if _ISMOBILE == 1
        texture.Update(position, flipX, flipY, GL_FILL);   
    #else
        texture.Update(position, flipX, flipY, 1); 
    #endif

    //update physics bodies if exists

    if (bodies.size())
        for (int i = 0; i < bodies.size(); i++)
            if (bodies[i].first->IsEnabled()) 
            {
                if (i == 0 && bodies[i].first->GetType() == b2_dynamicBody) 
                { 
                    b2Vec2 position = bodies[0].first->GetPosition(); 
                    position.y += bodies[0].second.w; //apply y offset
                    SetPosition(glm::vec2((position.x - bodies[0].second.x), ((position.y - bodies[0].second.y) - bodies[0].second.w)));
                }
                else
                    bodies[i].first->SetTransform(b2Vec2((position.x - bodies[0].second.x), ((position.y - bodies[0].second.y) - bodies[0].second.w)), 0);

            }

    //play current animation

    if (m_isSpritesheet && m_currentAnim.first.length())
        Animate(m_currentAnim.first, m_currentAnim.second.first, m_currentAnim.second.second); 

}
 

//-------------------------------------- standard sprite / tile


Sprite::Sprite(const std::string& key, float x, float y, int frame, bool isTile): 
    Entity("sprite", x, y)
{   
    this->key = key;
    currentFrame = frame;    
    anims = System::Resources::Manager::GetAnimations(key);
    velocityX = 0.0f;
    velocityY = 0.0f;
    texture = Graphics::Texture2D::GetTexture(key);
    shader = Shader::GetShader("sprite");          
    
    #if DEVELOPMENT == 1
        if (!isTile)
            std::cout << "Sprite: " + key + " Created.\n"; 
    #endif
}


//------------------------------ cc


Sprite::Sprite(Sprite& sprite):
    Entity("sprite", sprite.position.x, sprite.position.y)
{
    key = sprite.key;
    currentFrame = sprite.currentFrame;    
    anims = System::Resources::Manager::GetAnimations(sprite.key);
    velocityX = sprite.velocityX;
    velocityY = sprite.velocityY;
    texture = Graphics::Texture2D::GetTexture(sprite.key);
    shader = Shader::GetShader("sprite");     

    #if DEVELOPMENT == 1
        std::cout << "Sprite: " + key + " Cloned.\n"; 
    #endif

}



//-------------------------------------- UI sprite

 
Sprite::Sprite(const std::string& key, const glm::vec2& position): 
    Entity("UI", position.x, position.y)
{
    this->key = key;
    texture = Graphics::Texture2D::GetTexture(key);
    shader = Shader::GetShader("UI");
    
    #if DEVELOPMENT == 1
        std::cout << "Sprite: UI " + key + " created.\n"; 
    #endif
}



//-------------------------------------------


Sprite::~Sprite() {

    #if DEVELOPMENT == 1
        if (strcmp(type, "tile") != 0)
            std::cout << "Sprite: " + key + " Destroyed.\n"; 
    #endif
}




 
