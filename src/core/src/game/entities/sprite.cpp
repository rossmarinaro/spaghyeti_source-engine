  
#include "../../app/app.h"
#include "../../resources/manager/manager.h"

//------------------------------------ velocity


void Sprite::SetVelocity(float velX, float velY) 
{ 
    if (!this->m_active)
        return;

    this->m_position.x += velX * System::Application::game->time->GetSeconds(); 
    this->m_position.y += velY * System::Application::game->time->GetSeconds(); 
};

void Sprite::SetVelocityX(float velX) 
{ 
    if (this->m_active)
        this->m_position.x += velX; //* System::Application::game->time->GetSeconds();     
};

void Sprite::SetVelocityY(float velY) 
{ 
    if (this->m_active)
        this->m_position.y += velY; //* System::Application::game->time->GetSeconds(); 
};


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
            std::cout << "Sprite: An attempt to load JSON failed, because JSON has not been enabled.\n";
        #endif

    }

    else if (strcmp(spritesheet, "raw") == 0)
        this->m_resourceData = System::Resources::Manager::GetRawSpritesheetData(this->m_key); 

    else
        return;
    
    this->m_isSpritesheet = true;
    this->m_frames = this->m_resourceData.size();

}



//---------------------- animate sprite


void Sprite::Animate(const std::string &animKey, bool yoyo, int rate)
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

        }
    }

    catch (std::runtime_error& err) { 
        std::cout << "Sprite: error playing animation: " << err.what() << "\n"; 
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
              currentFrameHeight = (float)this->m_resourceData[this->m_currentFrame][3];

        this->m_texture.FrameWidth = currentFrameWidth;
        this->m_texture.FrameHeight = currentFrameHeight;

        this->m_texture.U1 = (currentFrameX * currentFrameWidth) / this->m_texture.Width;   
        this->m_texture.U2 = ((currentFrameX + 1) * currentFrameWidth) / this->m_texture.Width;

        this->m_texture.V1 = (currentFrameY * currentFrameHeight) / this->m_texture.Height; 
        this->m_texture.V2 = ((currentFrameY + 1) * currentFrameHeight) / this->m_texture.Height; 

    }

    //sprite model transformations

    this->m_model = glm::mat4(1.0f); 

    //this->m_model = glm::translate(this->m_model, glm::vec3(this->m_position, 0.0f));  

    this->m_model = glm::translate(this->m_model, glm::vec3(0.5f * this->m_texture.FrameWidth + this->m_position.x, 0.5f * this->m_texture.FrameHeight + this->m_position.y, 0.0f)); 

    this->m_model = glm::scale(this->m_model, glm::vec3(this->m_scale, 1.0f));  
    
    if (this->m_rotation > 0)
        this->m_model = glm::rotate(this->m_model, glm::radians(this->m_rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

    this->m_model = glm::translate(this->m_model, glm::vec3(-0.5f * this->m_texture.FrameWidth - this->m_position.x, -0.5f * this->m_texture.FrameHeight - this->m_position.y, 0.0f));

    //update shaders and textures 

    this->m_shader.SetInt("image", 0, true);

    #ifndef __EMSCRIPTEN__
        this->m_shader.SetInt("repeat", this->m_texture.Repeat, true);
    #endif

    this->m_shader.SetFloat("alphaVal", this->m_alpha, true); 
    this->m_shader.SetVec3f("tint", this->m_tint, true);

    this->m_shader.SetMat4("model", this->m_model, true);

    this->m_texture.Update(this->m_position, this->m_flipX, this->m_flipY); 

    //update physics body if exists

    if (this->m_body.self != nullptr)
    {

        if (this->m_body.self->IsEnabled()) {
            b2Vec2 position = this->m_body.self->GetPosition();
            this->SetPosition(glm::vec2(position.x + this->m_body.offset.x, position.y + this->m_body.offset.y)); 
        }
    } 

    //play current animation

    if (this->m_isSpritesheet && this->currentAnim != nullptr)
        this->Animate(this->currentAnim); 


}


//-------------------------------------- standard sprite


Sprite::Sprite(const std::string &key, const glm::vec2 &position, int frame)
: 
    Entity(position),
        m_key(key),  
        m_currentFrame(frame),
        m_shader(Shader::GetShader("sprite")), 
        m_texture(Graphics::Texture2D::GetTexture(key)),
        m_anims(System::Resources::Manager::GetAnimations(key)),
        m_isSpritesheet(false)
{
 
    this->ReadSpritesheetData(); 

    std::cout << "Sprite: " + this->m_key + " Created.\n";

}


//-------------------------------------- UI sprite

 
Sprite::Sprite(const std::string &key, const glm::vec2 &position, const char* type)
: 
    Entity(position),
        m_key(key),  
        m_type(type),
        m_shader(Shader::GetShader(type)), 
        m_texture(Graphics::Texture2D::GetTexture(key)),
        m_isSpritesheet(false)

{ 

    this->ReadSpritesheetData(); 
        
    std::cout << "Sprite: UI " + this->m_key + " created.\n"; 
}







 
