#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/physics.h"


Physics::Physics():
    m_gravity(b2Vec2(0.0f, 500.0f)),
    m_world(m_gravity),
    m_flags(0)
{ 
    enableDebug = false;
    sleeping = true;
    setWarmStart = false; 
    continuous = true;
    subStep = false;
    clearForces = false;

    gravityX = m_gravity.x; 
    gravityY = m_gravity.y;

    m_flags += b2Draw::e_shapeBit;
    m_flags += b2Draw::e_jointBit;
    m_flags += b2Draw::e_aabbBit;
    m_flags += b2Draw::e_centerOfMassBit;

    LOG("Physics: world enabled."); 
} 


//----------------------------------


void Physics::SetGravity(float x, float y) {
    gravityX = x;
    gravityY = y;
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

    body.self = System::Application::game->physics->m_world.CreateBody(&body.def);

    body.fixtureDef.isSensor = isSensor;

    b2PolygonShape box;

    box.SetAsBox(width, height);       
    body.self->CreateFixture(&box, 0.0f); 

    System::Application::game->physics->m_active_bodies.emplace_back(body.self);

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
 
    body.self = System::Application::game->physics->m_world.CreateBody(&body.def);

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

    System::Application::game->physics->m_active_bodies.emplace_back(body.self); 
    
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
    if (m_active_bodies.size())
        for (const auto& body : m_active_bodies)
            DestroyBody(body);

    m_active_bodies.clear();

    LOG("Physics: bodies cleared.");
}


//-------------------------------


void Physics::Update()
{

    static double accumulator = 0.0;

    accumulator += System::Application::game->time->m_delta;

    while (accumulator >= System::Application::game->time->timeStep) {
        
        m_world.Step(System::Application::game->time->timeStep, s_velocityIterations, s_positionIterations);

        accumulator -= System::Application::game->time->timeStep;
    }

    #if DEVELOPMENT == 1 
        debug->SetFlags(m_flags);
    #endif

	m_world.SetAllowSleeping(sleeping);
	m_world.SetWarmStarting(setWarmStart);
	m_world.SetContinuousPhysics(continuous);
	m_world.SetSubStepping(subStep);
    m_world.SetAutoClearForces(clearForces);

    //cleanup removed bodies

    std::set<b2Body*>::iterator it = m_bodiesToRemove.begin();
    std::set<b2Body*>::iterator end = m_bodiesToRemove.end();

    for (; it != end; ++it) 
    {
        auto b = *it;
        auto b_it = std::find(m_active_bodies.begin(), m_active_bodies.end(), b);

        if (b_it != m_active_bodies.end()) 
            m_active_bodies.erase(b_it);

        if (b != nullptr) {
            m_world.DestroyBody(b);
            b = nullptr;
        }
    }

    m_bodiesToRemove.clear();
 
    m_world.SetGravity(b2Vec2(gravityX, gravityY));
}

