#if USE_JSON == 1
    #include <fstream>
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../shared/renderer.h"
#include "../../vendors/glm/gtc/matrix_transform.hpp" 

#include "../../vendors/UUID.hpp"
#include "../../vendors/box2d/include/box2d/box2d.h"

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"

//-------------------------------------- standard sprite / tile


Sprite::Sprite(const std::string& key, float x, float y, bool isSpawn, bool isTile): 
    Entity(SPRITE, x, y, isSpawn)
{   
    this->key = key;

    velocityX = 0.0f;
    velocityY = 0.0f; 

    SetTexture(key);

    const auto animations = System::Resources::Manager::GetAnimations(key);

    if (animations) 
        anims.insert(animations->begin(), animations->end());      

    if (isTile) {
        m_type = TILE; 
        return;
    }

    LOG("Sprite: \"" + key + "\" created. (generic)");
}


//------------------------------ clone


Sprite::Sprite(const Sprite& sprite):
    Entity(SPRITE, sprite.position.x, sprite.position.y)
{
    key = sprite.key;
    currentFrame = sprite.currentFrame; 
    velocityX = sprite.velocityX;
    velocityY = sprite.velocityY;   

    const auto animations = System::Resources::Manager::GetAnimations(key);

    if (animations) 
        anims.insert(animations->begin(), animations->end()); 

    SetTexture(sprite.key);    

    LOG("Sprite: \"" + key + "\" cloned."); 

}

 
//-------------------------------------- UI sprite

 
Sprite::Sprite(const std::string& key, const Math::Vector2& position): 
    Entity(UI, position.x, position.y)
{
    this->key = key; 
    
    SetTexture(key);
    SetShader("sprite");  
    
    LOG("Sprite: \"" + key + "\" created. (UI)"); 
}

  
//-------------------------------------------


Sprite::~Sprite() {
    if (m_type != TILE) {
        LOG("Sprite: \"" + key + "\" destroyed."); 
    }
}


//-------------------------------------------


std::shared_ptr<Sprite> Sprite::Clone() 
{
    const auto clone = std::make_shared<Sprite>(*this);

    clone->ReadSpritesheetData();

    System::Game::GetScene()->entities.push_back(clone); 

    if (m_bodies.size())
        for (int i = 0; i < m_bodies.size(); i++) 
        {
            const std::shared_ptr<Physics::Body> body = m_bodies[0].first;
            const Math::Vector4 offset = m_bodies[0].second;

            clone->AddBody(Physics::CreateBody(Physics::Body::Type::DYNAMIC, offset.r, offset.g, offset.b, offset.a, body->isSensor, body->pointer, body->density, body->friction, body->restitution), { offset.r, offset.g, offset.b, offset.a } );
        }
            
    return clone;
}


//----------------------------- set UI or world sprite


void Sprite::SetAsUI(bool isUI) 
{ 
    const auto _this = System::Game::GetScene()->GetEntity<Sprite>(ID, true);

    m_type = isUI ? UI : SPRITE; 
    
    if (m_type == UI) 
    {
        auto it = std::find_if(System::Game::GetScene()->entities.begin(), System::Game::GetScene()->entities.end(), [this](const auto e) { return e->ID == ID; });

        if (it != System::Game::GetScene()->entities.end()) {
            _this->render_layer = 1;
            System::Game::GetScene()->UI.emplace_back(_this);
            it = System::Game::GetScene()->entities.erase(std::move(it));
            --it;
        }
    }
    else if (m_type == SPRITE)
    {
        auto it = std::find_if(System::Game::GetScene()->UI.begin(), System::Game::GetScene()->UI.end(), [this](const auto e) { return e->ID == ID; });

        if (it != System::Game::GetScene()->UI.end()) {
            System::Game::GetScene()->entities.emplace_back(_this);
            it = System::Game::GetScene()->UI.erase(std::move(it));
            --it;
        }
    }
}


//------------------------------------ velocity, use physics if available / else default to position update


void Sprite::SetVelocity(float velX, float velY) 
{ 
    if (!active)
        return;

    if (m_bodies.size()) 
    {
        velocityX = velX;
        velocityY = velY;

        const std::shared_ptr<Physics::Body> body = m_bodies[0].first;

        if (body->GetType() == Physics::Body::Type::DYNAMIC)
            body->SetLinearVelocity(velocityX, velocityY);
        else if (body->GetType() == Physics::Body::Type::KINEMATIC) {
            const float posX = body->GetPosition().x,
                        posY = body->GetPosition().y;
            body->SetTransform((posX + velocityX), (posY + velocityY));
        }
    }

    else {
        velocityX = velX / 1000.0f;
        velocityY = velY / 1000.0f;
    }
}


//--------------------------------


void Sprite::SetVelocityX(float velX) 
{ 
    if (!active)
        return;

    if (m_bodies.size()) 
    {
        velocityX = velX; 

        const std::shared_ptr<Physics::Body> body = m_bodies[0].first;

        if (body->GetType() == Physics::Body::Type::DYNAMIC)
            body->SetLinearVelocity(velocityX, body->GetLinearVelocity().y);

        else if (body->GetType() == Physics::Body::Type::KINEMATIC) {
            const float posX = body->GetPosition().x;
            body->SetTransform((posX + velocityX), body->GetPosition().y);
        }
    }

    else
        velocityX = velX / 1000.0f;    
}


//---------------------------------


void Sprite::SetVelocityY(float velY) 
{ 
    if (!active)
        return;
         
    if (m_bodies.size()) 
    {
        velocityY = velY;

        const std::shared_ptr<Physics::Body> body = m_bodies[0].first;

        if (body->GetType() == Physics::Body::Type::DYNAMIC)
            body->SetLinearVelocity(body->GetLinearVelocity().x, velocityY);
            
        else if (body->GetType() == Physics::Body::Type::KINEMATIC) {
            const float posY = body->GetPosition().y;
            body->SetTransform(body->GetLinearVelocity().x, (posY + velocityY));
        }
    }

    else
        velocityY = velY / 1000.0f;  
}


//----------------------------- set impulse x


void Sprite::SetImpulse(float x, float y) 
{
    if (active) 
    {
        if (m_bodies.size()) {
            const std::shared_ptr<Physics::Body> body = m_bodies[0].first;
            body->GetType() == Physics::Body::Type::DYNAMIC;
            body->ApplyLinearImpulse(x * 10000, y * 10000);
        }
        else 
            SetVelocity(x, y);
    }
}


//----------------------------- set impulse x


void Sprite::SetImpulseX(float x) 
{
    if (active) 
    {
        if (m_bodies.size()) {
            const std::shared_ptr<Physics::Body> body = m_bodies[0].first;
            body->GetType() == Physics::Body::Type::DYNAMIC;
            body->ApplyLinearImpulse(x * 10000, body->GetLinearVelocity().y);
        }
        else 
            SetVelocityX(x);
    }
}


//----------------------------- set impulse y


void Sprite::SetImpulseY(float y) 
{
    if (active) 
    {
        if (m_bodies.size()) {
            const std::shared_ptr<Physics::Body> body = m_bodies[0].first;
            body->GetType() == Physics::Body::Type::DYNAMIC;
            body->ApplyLinearImpulse(body->GetLinearVelocity().x, y * 10000);
        }
        else 
            SetVelocityY(y);
    }
}


//----------------------------- physics bodies


void Sprite::RemoveBodies() 
{
    //reset texture position to normal coords

    const float x = (m_bodies[0].first->GetPosition().x / scale.x) - m_bodies[0].second.r,
                y = (m_bodies[0].first->GetPosition().y / scale.y) - m_bodies[0].second.g;

    for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) 
        Physics::DestroyBody((*it).first);

    m_bodies.clear();

    SetPosition(x, y);
}


//----------------------------- 

void Sprite::AddBody(const std::shared_ptr<Physics::Body>& body, const Math::Vector4& offsets) { 
    std::pair<std::shared_ptr<Physics::Body>, Math::Vector4> b = { body, offsets };
    m_bodies.emplace_back(b);
}


//----------------------------- 



std::shared_ptr<Physics::Body> Sprite::GetBody(int index) { 
    const auto body = m_bodies.at(index).first;
    return body ? body : nullptr; 
}


//----------------------------- 



std::vector<std::pair<std::shared_ptr<Physics::Body>, Math::Vector4>>& Sprite::GetBodies() {
    return m_bodies;
} 



//----------------------------- set texture


void Sprite::SetTexture(const std::string& key)
{  
    if (System::Application::resources->textures.find(key) == System::Application::resources->textures.end()) 
        return;

    this->key = key; 
    texture = Graphics::Texture2D::Get(key); 
    m_isSpritesheet = false;
    m_resourceData.clear();
    m_currentAnim = { "", false, 0, 0, 0 };
    frames = 0;
    currentFrame = 0;
}


//-----------------------------


void Sprite::SetStroke(bool isOutlined, const Math::Vector3& color, float width) 
{
    outlineEnabled = isOutlined;

    if (outlineEnabled) {
        outlineWidth = width;
        outlineColor = color;
    }

    SetShader(outlineEnabled ? "outline sprite" : "sprite");
    
}


//----------------------------- read frame data


void Sprite::ReadSpritesheetData()
{    
    const auto spritesheet = System::Resources::Manager::GetSpritesheetPath(key);
    const auto rawSpritesheetData = System::Resources::Manager::GetRawSpritesheetData(key);

    //json file

    if (spritesheet && System::Utils::str_endsWith(*spritesheet, ".json")) 
    {
        #if USE_JSON == 1 

            std::ifstream JSON(*spritesheet);

            const json data = json::parse(JSON);

                for (const auto& frame : data["frames"])
                    if (frame.contains("frame")) 
                    {
                        const unsigned int x = frame["frame"]["x"],
                                           y = frame["frame"]["y"],
                                           w = frame["frame"]["w"],
                                           h = frame["frame"]["h"];

                        m_resourceData.push_back({ x, y, w, h });
                    }

            frames = m_resourceData.size();

        #else 
            LOG("Sprite: an attempt to load JSON failed, because JSON has not been enabled.");
        #endif

    }

    //int array

    else if (rawSpritesheetData) {
        frames = rawSpritesheetData->size();
        m_resourceData = *rawSpritesheetData; 
    } 

    //not a spritesheet

    else 
        return;
    
    m_isSpritesheet = true;

}


//------------------------------------------ animations


void Sprite::SetAnimation(const std::string& key, bool yoyo, int rate, int repeat) { 
    m_animComplete = false;
    m_currentAnim.key = key;
    m_currentAnim.rate = rate;
    m_currentAnim.repeat = repeat;
    m_currentAnim.yoyo = yoyo;
}
		

//------------------------------------------ 

void Sprite::StopAnimation() { 
    m_animComplete = false;
    m_currentAnim.key = "";
}


//------------------------------------------


void Sprite::Update()
{
    //update physics bodies if any

    for (const auto& body : m_bodies)
        if (body.first->IsEnabled() &&  
            body.first->GetType() == Physics::Body::Type::DYNAMIC || 
            body.first->GetType() == Physics::Body::Type::KINEMATIC
        ) {
            Math::Vector2 pos = body.first->GetPosition(); 
            SetPosition((pos.x / scale.x) - body.second.r, (pos.y / scale.y) - body.second.g);
        }
}

//------------------------------------------ render sprite / update transformations


void Sprite::Render()
{  
    if (!alive)
        return;

    //update spritesheet UV subtexturing if applicable

    if (m_isSpritesheet) 
    {
        const float currentFrameX = (float)m_resourceData[currentFrame][0],
                    currentFrameY = (float)m_resourceData[currentFrame][1],
                    currentFrameWidth = (float)m_resourceData[currentFrame][2],
                    currentFrameHeight = (float)m_resourceData[currentFrame][3],
                    factorX = (float)m_resourceData[currentFrame][4],
                    factorY = (float)m_resourceData[currentFrame][5];

        texture.FrameWidth = currentFrameWidth;
        texture.FrameHeight = currentFrameHeight;

        //texture coordinates for tiles are varying min and max uvs for texture extrusion

        texture.U1 = m_type == TILE ? (currentFrameX * currentFrameWidth) / texture.Width : (currentFrameX * factorX) / texture.Width;      
        texture.U2 = m_type == TILE ? ((currentFrameX + 1) * currentFrameWidth) / texture.Width : ((currentFrameX + currentFrameWidth) * factorX) / texture.Width;
        texture.V1 = m_type == TILE ? (currentFrameY * currentFrameHeight) / texture.Height : (currentFrameY * factorY) / texture.Height; 
        texture.V2 = m_type == TILE ? ((currentFrameY + 1) * currentFrameHeight) / texture.Height : ((currentFrameY + currentFrameHeight) * factorY) / texture.Height; 
    }

    //sprite model rotation transformation

    glm::mat4 identityMatrix = glm::mat4(1.0f); 

    identityMatrix = glm::translate(identityMatrix, { 0.5f * texture.FrameWidth + position.x, 0.5f * texture.FrameHeight + position.y, 0.0f }); 
    identityMatrix = glm::rotate(identityMatrix, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); 
    identityMatrix = glm::scale(identityMatrix, { scale.x, scale.y, 1.0f });
    identityMatrix = glm::translate(identityMatrix, { -0.5f * texture.FrameWidth - position.x, -0.5f * texture.FrameHeight - position.y, 0.0f });

    //update texture

    if (active && renderable)
    {
        const auto camera = System::Application::game->camera;

        float scrollX = scrollFactor.x, 
              scrollY = scrollFactor.y;

        #if STANDALONE == 0
            scrollX = 1.0f; 
            scrollY = 1.0f;
        #endif

        const Math::Vector4& pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
        const Math::Matrix4& vm = camera->GetViewMatrix((camera->GetPosition()->x * scrollX), (camera->GetPosition()->y * scrollY));
        const glm::highp_mat4 projMat = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f); 
    
        glm::mat4 mvp = projMat * glm::mat4({ vm.a.r, vm.a.g, vm.a.b, vm.a.a }, 
                                { vm.b.r, vm.b.g, vm.b.b, vm.b.a }, 
                                { vm.c.r, vm.c.g, vm.c.b, vm.c.a }, 
                                { vm.d.r, vm.d.g, vm.d.b, vm.d.a }) * identityMatrix;
   
        if (!IsSprite()) //UI do not have view matrix
            mvp = projMat * glm::mat4(1.0f) * identityMatrix;

        const float r = tint.x, 
                    g = tint.y, 
                    b = tint.z;

        float whiteout = 0.0f; 

        if (texture.Whiteout) 
            whiteout = 1.0f;

        const Math::Vector4 color = { r, g, b, alpha };

        const Math::Matrix4 modelViewProj = { 
            { mvp[0][0], mvp[0][1], mvp[0][2], mvp[0][3] }, 
            { mvp[1][0], mvp[1][1], mvp[1][2], mvp[1][3] },   
            { mvp[2][0], mvp[2][1], mvp[2][2], mvp[2][3] },  
            { mvp[3][0], mvp[3][1], mvp[3][2], mvp[3][3] }
        };

        //update texture

        texture.Update(
            shader, 
            position, 
            color, 
            outlineColor,
            modelViewProj, 
            outlineEnabled ? outlineWidth : 0.0f, 
            whiteout,
            depth, 
            flipX, 
            flipY
        );  

        const auto renderer = System::Renderer::Get();

        if (renderer)
            renderer->drawStyle = 1;
    }

    //play current animation

    if (m_isSpritesheet && m_currentAnim.key.length())
    {
        const std::string& animKey = m_currentAnim.key;
        const bool yoyo = m_currentAnim.yoyo;
        const int rate = m_currentAnim.rate;

        const uint32_t seconds = System::Application::game->time->GetSeconds() * rate;

        try {

            if (m_isSpritesheet && active)
            {
                const auto anim = anims.find(animKey.c_str());

                if (anim == anims.end() || System::Game::GetScene()->IsPaused() || ((m_currentAnim.repeat <= 0 && m_currentAnim.repeat != -1))) 
                    return;

                std::vector<int> frames; //frames to populate  
                
                int startFrame = anims.find(m_currentAnim.key.c_str())->second.first,
                    endFrame = anims.find(m_currentAnim.key.c_str())->second.second,
                    frame = yoyo ? startFrame : endFrame;

                m_animComplete = currentFrame == frame && m_currentAnim.can_complete;
                m_currentAnim.can_complete = false;

                if (yoyo)
                {
                    for (int i = anim->second.first; i < anim->second.second + 1; i++) 
                        frames.emplace_back(i);
                    
                    const uint32_t elapsed = seconds % frames.size();

                    std::vector<int> frames_reversed;

                    for (int i = anim->second.second; i > anim->second.first - 1; i--) 
                        frames_reversed.emplace_back(i);

                    const uint32_t elapsed_reversed = seconds % frames_reversed.size();

                    if (!m_anim_yoyo && currentFrame == anim->second.second && frames[elapsed] != anim->second.second) 
                        m_anim_yoyo = true;

                    if (m_anim_yoyo && currentFrame == anim->second.first && frames_reversed[elapsed_reversed] != anim->second.first)    
                        m_anim_yoyo = false;

                    if (m_anim_yoyo) 
                        SetFrame(frames_reversed[elapsed_reversed]);

                    else  
                        SetFrame(frames[elapsed]);
                }

                else {
                    for (int i = anim->second.first; i < anim->second.second + 1; i++) 
                        frames.emplace_back(i);
                    
                    const uint32_t elapsed = seconds % frames.size();
                
                    SetFrame(frames[elapsed]);
                }

                //animation complete

                m_currentAnim.can_complete = true;

                if (currentFrame == anim->second.second) 
                {
                    if (m_currentAnim.can_decrement) {
                        m_currentAnim.can_decrement = false;

                        if (m_currentAnim.repeat > -1) 
                            m_currentAnim.repeat--; 
                    }
                }
                else 
                    m_currentAnim.can_decrement = true;
            }
        }

        catch (std::runtime_error& err) { 
            LOG("Sprite: error playing animation: " + (std::string)err.what()); 
        }
    }
}
 


//----------------------------------- (sprite overlap not box2d related)


const bool Sprite::CheckOverlap(const std::shared_ptr<Sprite>& spriteA, const std::shared_ptr<Sprite>& spriteB)
{
    //x axis

    const bool collisionX = spriteA->position.x + spriteA->texture.FrameWidth / 2 >= spriteB->position.x &&
                            spriteB->position.x + spriteB->texture.FrameWidth / 2 >= spriteA->position.x,

    //y axis

               collisionY = spriteA->position.y + spriteA->texture.FrameHeight / 2 >= spriteB->position.y &&
                            spriteB->position.y + spriteB->texture.FrameHeight / 2 >= spriteA->position.y;

    return collisionX && collisionY;

}
 
//-----------------------------


std::shared_ptr<Sprite> System::Game::CreateSprite(const std::string& key, float x, float y, int frame, float scale, int layer, bool isSpawn)
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


//----------------------------- ui sprite default layer 1


std::shared_ptr<Sprite> System::Game::CreateUISprite(const std::string& key, float x, float y, int frame, float scale)
{
    const Math::Vector2 pos = { x, y };

    const auto element = std::make_shared<Sprite>(key, pos);

    GetScene()->UI.emplace_back(element);

    #if STANDALONE == 1
        element->ReadSpritesheetData();
        element->SetFrame(frame);
    #endif

    element->SetScale(scale);
    element->render_layer = 1;

    return element;
}