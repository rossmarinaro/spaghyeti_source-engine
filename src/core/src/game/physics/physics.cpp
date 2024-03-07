
#include "../../../../../build/include/app.h"
#include "../../../../../build/include/physics.h"

//------------------------------

Physics::Physics():
    gravityX(0.0f),
    gravityY(500.0f),
    gravity(b2Vec2(this->gravityX, this->gravityY)), 
    world(gravity)  
{ 

    m_flags = 0;

    m_flags += b2Draw::e_shapeBit;
    m_flags += b2Draw::e_jointBit;
    m_flags += b2Draw::e_aabbBit;
    m_flags += b2Draw::e_centerOfMassBit;

    std::cout << "Physics: world enabled.\n"; 
} 


//----------------------------------


b2Body* Physics::CreateStaticBody(
    float x, 
    float y, 
    float width, 
    float height, 
    bool isSensor, 
    int type
)
{

    Body body;

    body.def.type = b2_staticBody;
    body.def.position.Set(x, y);     

    body.def.userData.pointer = type;

    body.self = System::Application::game->physics->world.CreateBody(&body.def);

    body.m_width = width;
    body.m_height = height;

    body.fixtureDef.isSensor = isSensor;

    body.box.SetAsBox(body.m_width, body.m_height);       
    body.self->CreateFixture(&body.box, 0.0f); 

    return body.self;
}



//------------------------------------


b2Body* Physics::CreateDynamicBody(
    float x,
    float y,
    float width,
    float height,
    bool isSensor,
    int type, 
    float density, 
    float friction, 
    float restitution
)
{

    Body body;

    body.def.type = b2_dynamicBody;
  
    body.def.position.Set(x, y);
 
    body.def.userData.pointer = type;
 
    body.self = System::Application::game->physics->world.CreateBody(&body.def);  
    
    body.box.SetAsBox(width, height);          

    body.m_width = width;
    body.m_height = height;

    body.fixtureDef.shape = &body.box; 
    body.fixtureDef.density = density;  
    body.fixtureDef.friction = friction; 
    body.fixtureDef.restitution = restitution;
    body.fixtureDef.isSensor = isSensor;

    body.self->CreateFixture(&body.fixtureDef);
    
    return body.self;
}


//-----------------------------


//does not destroy body immediately. body will be destroyed after next timestep
void Physics::DestroyBody(b2Body* b) {
    System::Application::game->physics->bodiesToRemove.insert(b);
}


//-------------------------------


void Physics::Update()
{
    //cleanup removed bodies

    std::set<b2Body*>::iterator it = System::Application::game->physics->bodiesToRemove.begin();
    std::set<b2Body*>::iterator end = System::Application::game->physics->bodiesToRemove.end();

    for (; it != end; ++it) 
    {
        auto b = *it;

        if (b != nullptr) {
            world.DestroyBody(b);
            b = nullptr;
        }
    }

    System::Application::game->physics->bodiesToRemove.clear();

    //if (System::Application::game->physics.world != nullptr)
     //   world.SetGravity(b2Vec2(gravityX, gravityY));
}

