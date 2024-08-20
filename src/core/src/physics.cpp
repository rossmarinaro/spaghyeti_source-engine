#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/physics.h"


Physics::Physics():
    m_gravity(b2Vec2(0.0f, 500.0f)),
    m_world(m_gravity),
    m_flags(0)
{ 
    this->gravityX = this->m_gravity.x; 
    this->gravityY = this->m_gravity.y;

    this->m_flags += b2Draw::e_shapeBit;
    this->m_flags += b2Draw::e_jointBit;
    this->m_flags += b2Draw::e_aabbBit;
    this->m_flags += b2Draw::e_centerOfMassBit;

    #if DEVELOPMENT == 1
        LOG("Physics: world enabled."); 
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

    body.self = System::Application::game->physics->m_world.CreateBody(&body.def);

    body.fixtureDef.isSensor = isSensor;

    b2PolygonShape box;

    box.SetAsBox(width, height);       
    body.self->CreateFixture(&box, 0.0f); 

    System::Application::game->physics->active_bodies.push_back(body.self);

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

    System::Application::game->physics->active_bodies.push_back(body.self); 
    
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
    if (this->active_bodies.size())
        for (const auto& body : this->active_bodies)
            DestroyBody(body);

    this->active_bodies.clear();
}


//-------------------------------


void Physics::Update()
{

    static double accumulator = 0.0;

    accumulator += System::Application::game->time->m_delta;

    while (accumulator >= System::Application::game->time->timeStep) {
        
        this->m_world.Step(System::Application::game->time->timeStep, s_velocityIterations, s_positionIterations);

        accumulator -= System::Application::game->time->timeStep;
    }

    #if DEVELOPMENT == 1 
        this->debug->SetFlags(this->m_flags);
    #endif

	this->m_world.SetAllowSleeping(this->sleeping);
	this->m_world.SetWarmStarting(this->setWarmStart);
	this->m_world.SetContinuousPhysics(this->continuous);
	this->m_world.SetSubStepping(this->subStep);
    this->m_world.SetAutoClearForces(this->clearForces);

    //cleanup removed bodies

    std::set<b2Body*>::iterator it = this->m_bodiesToRemove.begin();
    std::set<b2Body*>::iterator end = this->m_bodiesToRemove.end();

    for (; it != end; ++it) 
    {
        auto b = *it;
        auto b_it = std::find(this->active_bodies.begin(), this->active_bodies.end(), b);

        if (b_it != this->active_bodies.end()) 
            this->active_bodies.erase(b_it);

        if (b != nullptr) {
            this->m_world.DestroyBody(b);
            b = nullptr;
        }
    }

    this->m_bodiesToRemove.clear();
 
    this->m_world.SetGravity(b2Vec2(this->gravityX, this->gravityY));
}

