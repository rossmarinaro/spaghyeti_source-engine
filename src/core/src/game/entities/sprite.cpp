  
#include "../../../../../build/sdk/include/app.h"
#include "../../../../../build/sdk/include/manager.h"


//------------------------------------ velocity, use physics if available / else default to position update


void Sprite::SetVelocity(float velX, float velY) 
{ 

    if (!this->m_active)
        return;

    this->velocityX = velX;
    this->velocityY = velY;

    if (this->bodies.size()) 
        this->bodies[0].first->SetLinearVelocity(b2Vec2(this->velocityX, this->velocityY));

    else {
        this->m_position.x += this->velocityX /* * System::Application::game->time->GetSeconds() */; 
        this->m_position.y += this->velocityY /* * System::Application::game->time->GetSeconds() */; 
    }

}


//--------------------------------


void Sprite::SetVelocityX(float velX) 
{ 

    if (!this->m_active)
        return;

    this->velocityX = velX; 

    if (this->bodies.size() && this->IsContacting()) 
        this->bodies[0].first->SetLinearVelocity(b2Vec2(this->velocityX, this->bodies[0].first->GetLinearVelocity().y));

    else
        this->m_position.x += this->velocityX; // System::Application::game->time->GetSeconds();     
}


//---------------------------------


void Sprite::SetVelocityY(float velY) 
{ 

    if (!this->m_active)
        return;

    this->velocityY = velY;

    if (this->bodies.size()) 
        this->bodies[0].first->SetLinearVelocity(b2Vec2(this->bodies[0].first->GetLinearVelocity().x, this->velocityY));
    
    else
        this->m_position.y += this->velocityY; // System::Application::game->time->GetSeconds(); 
}


//----------------------------- set impulse x


void Sprite::SetImpulse(float x, float y) {

    if (this->m_active && this->bodies.size())
        this->bodies[0].first->ApplyLinearImpulse(b2Vec2(x * 10000, y * 10000), this->bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- set impulse x


void Sprite::SetImpulseX(float x) {

    if (this->m_active && this->bodies.size())
        this->bodies[0].first->ApplyLinearImpulse(b2Vec2(x * 10000, this->bodies[0].first->GetLinearVelocity().y), this->bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- set impulse y


void Sprite::SetImpulseY(float y) {

    if (this->m_active && this->bodies.size())
        this->bodies[0].first->ApplyLinearImpulse(b2Vec2(this->bodies[0].first->GetLinearVelocity().x, y * 10000), this->bodies[0].first->GetWorldCenter(), true);
}


//----------------------------- remove bodies


void Sprite::RemoveBodies() 
{

    //reset texture position to normal coords

    float x = this->bodies[0].first->GetPosition().x / 2,
          y = this->bodies[0].first->GetPosition().y / 2;

    for (auto it = this->bodies.begin(); it != this->bodies.end(); ++it) 
        Physics::DestroyBody((*it).first);

    this->bodies.clear();

    this->SetPosition(x, y);
}


//----------------------------- set texture


void Sprite::SetTexture(const std::string& key)
{  

    auto texture = this->m_texture.GetTexture(key); 
        
    this->m_key = key; 
    //this->m_texture.Width = texture.Width;        
    //this->m_texture.Height = texture.Height;
    this->m_texture.FrameWidth = texture.Width;
    this->m_texture.FrameHeight = texture.Height;
    this->m_texture.ID = texture.ID;

    this->currentAnim = {};
}


//----------------------------- read animation data


void Sprite::ReadSpritesheetData()
{    
    
    const char* spritesheet = System::Resources::Manager::GetSpritesheetPath(this->m_key);
 
    //json file

    if (System::Utils::str_includes(spritesheet, ".json")) 
    {

        #if USE_JSON == 1 
            std::ifstream JSON(spritesheet);

            json data = json::parse(JSON);

                for (auto &array : data)
                    for (auto &element : array)
                            this->m_resourceData.push_back({ element["x"], element["y"], element["w"], element["h"] });
        #else 
            #if DEVELOPMENT == 1
                std::cout << "Sprite: An attempt to load JSON failed, because JSON has not been enabled.\n";
            #endif
        #endif

    }

    else if (System::Resources::Manager::GetRawSpritesheetData(this->m_key).size())
        this->m_resourceData = System::Resources::Manager::GetRawSpritesheetData(this->m_key);  

    else
        return;
    
    this->m_isSpritesheet = true;
    this->m_frames = this->m_resourceData.size();

}



//---------------------- animate sprite


void Sprite::Animate(const std::string& animKey, bool yoyo, int rate)
{ 

    uint32_t seconds = System::Application::game->time->GetSeconds() * rate;

    try {

        if (this->m_isSpritesheet && this->m_active)
        {

            std::map<std::string, std::pair<int, int>>::iterator anim = this->m_anims.find(animKey);

            if (anim == this->m_anims.end()) 
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

                if (!this->anim_yoyo && this->m_currentFrame == anim->second.second && frames[elapsed] != anim->second.second) 
                    this->anim_yoyo = true;

                if (this->anim_yoyo && this->m_currentFrame == anim->second.first && frames_reversed[elapsed_reversed] != anim->second.first)    
                    this->anim_yoyo = false;

                if (this->anim_yoyo) 
                    this->SetFrame(frames_reversed[elapsed_reversed]);

                else 
                    this->SetFrame(frames[elapsed]);
             
            }

            else    
            {
                for (int i = anim->second.first; i < anim->second.second; i++) 
                    frames.push_back(i);
                
                uint32_t elapsed = seconds % frames.size();

                this->SetFrame(m_currentFrame != anim->second.second ? frames[elapsed] : anim->second.first);
            }

            this->currentAnim = { animKey, { yoyo, rate } };

            int frame = yoyo ? 
                this->m_anims.find(this->currentAnim.first)->second.first : 
                this->m_anims.find(this->currentAnim.first)->second.second;

			this->m_animComplete = frame == this->m_currentFrame;

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

    if (!this->m_renderable || !this->m_alive)
        return;

    //update spritesheet UV subtexturing if applicable

    if (this->m_isSpritesheet) 
    {
        
        float currentFrameX = (float)this->m_resourceData[this->m_currentFrame][0],
              currentFrameY = (float)this->m_resourceData[this->m_currentFrame][1],
              currentFrameWidth = (float)this->m_resourceData[this->m_currentFrame][2],
              currentFrameHeight = (float)this->m_resourceData[this->m_currentFrame][3],
              factorX = (float)this->m_resourceData[this->m_currentFrame][4],
              factorY = (float)this->m_resourceData[this->m_currentFrame][5];

        this->m_texture.FrameWidth = currentFrameWidth;
        this->m_texture.FrameHeight = currentFrameHeight;

        //tilemap

        if (strcmp(this->type, "tile") == 0)
        {
            this->m_texture.U1 = (currentFrameX * currentFrameWidth) / this->m_texture.Width;      
            this->m_texture.U2 = ((currentFrameX + 1) * currentFrameWidth) / this->m_texture.Width;

            this->m_texture.V1 = (currentFrameY * currentFrameHeight) / this->m_texture.Height; 
            this->m_texture.V2 = ((currentFrameY + 1) * currentFrameHeight) / this->m_texture.Height; 
        }

        //generic

        else 
        {
            this->m_texture.U1 = (currentFrameX * factorX) / this->m_texture.Width;      
            this->m_texture.U2 = ((currentFrameX + currentFrameWidth) * factorX) / this->m_texture.Width;

            this->m_texture.V1 = (currentFrameY * factorY) / this->m_texture.Height; 
            this->m_texture.V2 = ((currentFrameY + currentFrameHeight) * factorY) / this->m_texture.Height; 
        }

    }

    //sprite model transformations

    this->m_model = glm::mat4(1.0f);       

    this->m_model = glm::translate(this->m_model, { 0.5f * this->m_texture.FrameWidth + this->m_position.x, 0.5f * this->m_texture.FrameHeight + this->m_position.y, 0.0f }); 
    
    //apply scaling to sprites that have bodies
    
    if (this->bodies.size())
        this->m_model = glm::scale(this->m_model, glm::vec3(this->m_scale, 1.0f));   

    this->m_model = glm::rotate(this->m_model, glm::radians(this->m_rotation), { 0.0f, 0.0f, 1.0f }); 
    this->m_model = glm::translate(this->m_model, { -0.5f * this->m_texture.FrameWidth - this->m_position.x, -0.5f * this->m_texture.FrameHeight - this->m_position.y, 0.0f });

    //update shaders and textures 

   // if (strcmp(this->type, "tile") == 0)
        //this->m_shader.SetInt("images", 1, true);
    //else
        this->m_shader.SetInt("image", 0, true);

    #ifndef __EMSCRIPTEN__
        this->m_shader.SetInt("repeat", this->m_texture.Repeat, true);
    #endif

    this->m_shader.SetVec2f("scale", glm::vec2(
        this->bodies.size() ? 1.0f : this->m_scale.x, 
        this->bodies.size() ? 1.0f : this->m_scale.y
    ), true); 
    
    this->m_shader.SetFloat("alphaVal", this->m_alpha, true); 
    this->m_shader.SetVec3f("tint", this->m_tint, true);
    this->m_shader.SetMat4("model", this->m_model, true);

    this->m_texture.Update(this->m_position, this->m_flipX, this->m_flipY, GL_FILL);   

    //update physics bodies if exists

    if (this->bodies.size())
        for (int i = 0; i < this->bodies.size(); i++)
            if (this->bodies[i].first->IsEnabled()) 
            {
                if (i == 0 && this->bodies[i].first->GetType() == b2_dynamicBody) 
                { 
                    b2Vec2 position = this->bodies[0].first->GetPosition(); 
                    position.y += this->bodies[0].second.w; //apply y offset

                    this->SetPosition(glm::vec2(position.x - this->bodies[0].second.x, position.y - this->bodies[0].second.y - this->bodies[0].second.w)); 
                }
                else
                    this->bodies[i].first->SetTransform(b2Vec2(this->m_position.x + this->bodies[i].second.x, this->m_position.y + this->bodies[i].second.y), 0);

            }

    //play current animation

    if (this->m_isSpritesheet && this->currentAnim.first.length())
        this->Animate(this->currentAnim.first, this->currentAnim.second.first, this->currentAnim.second.second); 

}

//-------------------------------------- standard sprite / tile


Sprite::Sprite(const std::string& key, float x, float y, int frame, bool isTile)
: 
    Entity("sprite", x, y),
        m_key(key),
        m_currentFrame(frame),    
        m_anims(System::Resources::Manager::GetAnimations(key)),
        velocityX(0.0f),
        velocityY(0.0f)
{ 

    this->m_texture = Graphics::Texture2D::GetTexture(key);
    this->m_shader = Shader::GetShader("sprite");

    #if DEVELOPMENT == 1
        if (!isTile)
            std::cout << "Sprite: " + this->m_key + " Created.\n"; 
    #endif
}



//-------------------------------------- UI sprite

 
Sprite::Sprite(const std::string& key, const glm::vec2& position)
: 
    Entity("UI", position.x, position.y),
        m_key(key)
{

    this->m_texture = Graphics::Texture2D::GetTexture(key);
    this->m_shader = Shader::GetShader("UI");
    
    #if DEVELOPMENT == 1
        std::cout << "Sprite: UI " + this->m_key + " created.\n"; 
    #endif
}



//-------------------------------------------


Sprite::~Sprite() 
{

    #if DEVELOPMENT == 1
        if (strcmp(this->type, "tile") != 0)
            std::cout << "Sprite: " + this->m_key + " Destroyed.\n"; 
    #endif
}




 
