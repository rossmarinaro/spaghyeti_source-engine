
#include "../../../../../build/sdk/include/app.h"
#include "../../../../../build/sdk/include/physics.h"

//------------------------------

Physics::Physics():
    m_gravity(b2Vec2(this->gravityX, this->gravityY)), 
    gravityX(0.0f),
    gravityY(500.0f),
    world(m_gravity)  
{ 

    m_flags = 0;

    m_flags += b2Draw::e_shapeBit;
    m_flags += b2Draw::e_jointBit;
    m_flags += b2Draw::e_aabbBit;
    m_flags += b2Draw::e_centerOfMassBit;

    #if DEVELOPMENT == 1
        std::cout << "Physics: world enabled.\n"; 
    #endif
} 


//----------------------------------


b2Body* Physics::CreateStaticBody(
    float x, 
    float y, 
    float width, 
    float height, 
    bool isSensor, 
    int pointer
)
{

    Body body;

    body.def.type = b2_staticBody;
    body.def.position.Set(x, y);     

    body.def.userData.pointer = pointer;

    body.self = System::Application::game->physics->world.CreateBody(&body.def);

    body.fixtureDef.isSensor = isSensor;

    b2PolygonShape box;

    box.SetAsBox(width, height);       
    body.self->CreateFixture(&box, 0.0f); 

    System::Application::game->physics->m_active_bodies.push_back(body.self);

    return body.self;
}



//------------------------------------


b2Body* Physics::CreateDynamicBody(
    const std::string& type,
    float x,
    float y,
    float width,
    float height,
    bool isSensor,
    int pointer, 
    float density, 
    float friction, 
    float restitution
)
{

    Body body;

    body.def.type = b2_dynamicBody;
  
    body.def.position.Set(x, y);
 
    body.def.userData.pointer = pointer;
 
    body.self = System::Application::game->physics->world.CreateBody(&body.def);

    b2CircleShape circle;
    b2PolygonShape box;

    if (type == "circle") { 
	    circle.m_radius = 0.3f;
        body.fixtureDef.shape = &circle;
    }

    if (type == "box") {
        box.SetAsBox(width, height);          
        body.fixtureDef.shape = &box;
    }

    else {
        box.SetAsBox(10, 10);          
        body.fixtureDef.shape = &box;
    }
 
    body.fixtureDef.density = density;  
    body.fixtureDef.friction = friction; 
    body.fixtureDef.restitution = restitution;
    body.fixtureDef.isSensor = isSensor;

    body.self->CreateFixture(&body.fixtureDef);

    System::Application::game->physics->m_active_bodies.push_back(body.self); 
    
    return body.self;
}


//-----------------------------


//does not destroy body immediately. body will be destroyed after next timestep
void Physics::DestroyBody(b2Body* b) {
    System::Application::game->physics->m_bodiesToRemove.insert(b);
}


//------------------------------


void Physics::ClearBodies() 
{
    if (this->m_active_bodies.size())
        for (const auto& body : this->m_active_bodies)
            DestroyBody(body);
            
    this->m_active_bodies.clear();
}


//-------------------------------


void Physics::Update()
{

    static double accumulator = 0.0;

    accumulator += System::Application::game->time->m_delta;

    while (accumulator >= System::Application::game->time->timeStep) {
        
        world.Step(
            System::Application::game->time->timeStep, 
            System::Application::game->physics->velocityIterations, 
            System::Application::game->physics->positionIterations
        );

        accumulator -= System::Application::game->time->timeStep;
    }

    #if DEVELOPMENT == 1 
        System::Application::game->physics->debug->SetFlags(System::Application::game->physics->m_flags);
    #endif

	world.SetAllowSleeping(System::Application::game->physics->sleeping);
	world.SetWarmStarting(System::Application::game->physics->setWarmStart);
	world.SetContinuousPhysics(System::Application::game->physics->continuous);
	world.SetSubStepping(System::Application::game->physics->subStep);
    world.SetAutoClearForces(System::Application::game->physics->clearForces);

    //cleanup removed bodies

    std::set<b2Body*>::iterator it = System::Application::game->physics->m_bodiesToRemove.begin();
    std::set<b2Body*>::iterator end = System::Application::game->physics->m_bodiesToRemove.end();

    for (; it != end; ++it) 
    {
        auto b = *it;

        if (b != nullptr) {
            world.DestroyBody(b);
            b = nullptr;
        }
    }

    System::Application::game->physics->m_bodiesToRemove.clear();

    world.SetGravity(b2Vec2(gravityX, gravityY));
}

