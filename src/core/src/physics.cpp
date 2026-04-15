#include <set>

#include "../../vendors/box2d/include/box2d/box2d.h"
#include "../../vendors/UUID.hpp"

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/physics.h"


static std::set<std::pair<const std::string, b2Body*>> _bodiesToRemove;
static std::map<const std::string, std::pair<const std::shared_ptr<Physics::Body>, b2Body*>> _active_b2d_bodies;
static b2World* _world;
static int _bodyCount = 0;


//---------------------------------- return underlying box2d body pointer to be used internally


static b2Body* _GetBox2DBody(const std::string& id) {
    auto it = _active_b2d_bodies.find(id);

    if (it != _active_b2d_bodies.end())
        return it->second.second;
 
    return nullptr;
}


//----------------------------------


Physics::Physics()
{
    enableDebug = false;
    sleeping = true;
    setWarmStart = false;
    continuous = true;
    subStep = false;
    clearForces = false;

    b2Vec2 grav(gravityX, gravityY);

    _world = new b2World(grav);

    LOG("Physics: initialized.");
}


//----------------------------------


Physics::~Physics() 
{
    ClearBodies();

    delete _world;
    _world = nullptr;

    LOG("Physics: world destroyed.");
}



//------------------------------------


void* Physics::GetWorld() {
    return _world ? _world : nullptr;
}



//----------------------------------


void Physics::SetGravity(float x, float y) {
    gravityX = x;
    gravityY = y;
}



//------------------------------


void Physics::ClearBodies()
{
    if (_active_b2d_bodies.size())
        for (const auto& body : _active_b2d_bodies) { 
            _bodiesToRemove.insert({ body.first, body.second.second }); }

    _active_b2d_bodies.clear();

    Cleanup();

    LOG("Physics: bodies cleared.");
}


//-------------------------------


void Physics::Cleanup() 
{
    //destroy phys-body in set before clearing 

    for (auto it = _bodiesToRemove.begin(); it != _bodiesToRemove.end(); ++it)
    {
        auto body = *it;
        auto b_it = std::find_if(_active_b2d_bodies.begin(), _active_b2d_bodies.end(), [body](const auto& b) { return b.first == body.first; });

        if (b_it != _active_b2d_bodies.end()) {
            b_it = _active_b2d_bodies.erase(b_it);
            --b_it;
        }

        if (body.second != nullptr) {
            _world->DestroyBody(body.second);
            body.second = nullptr;
        }
    }

    _bodiesToRemove.clear(); 
}


//-------------------------------


void Physics::Update()
{
    if (!_world)
        return;

    static double accumulator = 0.0;

    accumulator += System::Application::game->time->delta;

    while (accumulator >= System::Application::game->time->timeStep) {
        _world->Step(System::Application::game->time->timeStep, s_velocityIterations, s_positionIterations);
        accumulator -= System::Application::game->time->timeStep;
    }

	_world->SetAllowSleeping(sleeping);
	_world->SetWarmStarting(setWarmStart);
	_world->SetContinuousPhysics(continuous);
	_world->SetSubStepping(subStep);
    _world->SetAutoClearForces(clearForces);

    //cleanup removed bodies

    Cleanup();
    
    _world->SetGravity(b2Vec2(gravityX, gravityY));
}


//------------------------------------


b2Body* InitBox2DBody(int physicsType, float x, float y, int pointer)
{
    b2BodyDef b2d_def;
    b2BodyType bodyType;

    if (physicsType == Physics::Body::Type::STATIC)
        bodyType = b2_staticBody;

    else if (physicsType == Physics::Body::Type::KINEMATIC)
        bodyType = b2_kinematicBody;

    else if (physicsType == Physics::Body::Type::DYNAMIC)
        bodyType = b2_dynamicBody;

    else {
        LOG("Physics: invalid body type argument, defaulting to static.");
        bodyType = b2_staticBody;
    }
    
    b2d_def.type = bodyType;
    b2d_def.userData.pointer = pointer <= -1 ? pointer : _bodyCount;
    b2d_def.position.Set(x, y);

    b2Body* b2d_body = _world->CreateBody(&b2d_def);

    if (pointer > -1)
        _bodyCount++;

    return b2d_body;
}


//------------------------------------ rect


std::shared_ptr<Physics::Body> Physics::CreateBody(
    int physicsType, 
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
    const auto body = std::make_shared<Physics::Body>(physicsType, x, y, width, height, pointer <= -1 ? pointer : _bodyCount, isSensor, density, friction, restitution);
    
    b2Body* b2d_body = InitBox2DBody(physicsType, x, y, pointer);

    body->CreateFixture(width, height, isSensor, density, friction, restitution); 

    _active_b2d_bodies.insert({ body->id, { body, b2d_body } });
    
    return body;
}


//------------------------------------ circle


std::shared_ptr<Physics::Body> Physics::CreateBody(
    int physicsType, 
    float x, 
    float y,  
    float radius, 
    bool isSensor, 
    int pointer, 
    float density, 
    float friction, 
    float restitution
)
{
    const auto body = std::make_shared<Physics::Body>(physicsType, x, y, radius, pointer <= -1 ? pointer : _bodyCount, isSensor, density, friction, restitution);

    b2Body* b2d_body = InitBox2DBody(physicsType, x, y, pointer);

    body->CreateFixture(radius, isSensor, density, friction, restitution); 

    _active_b2d_bodies.insert({ body->id, { body, b2d_body } });

    return body;
}



//-----------------------------


void Physics::DestroyBody(const std::shared_ptr<Body>& body) 
{
    const auto b2d_body = _GetBox2DBody(body->id);
    
    if (b2d_body) {
        _bodiesToRemove.insert({ body->id, b2d_body });   
        _bodyCount--; 
    }
}


/* Body */


//------------------------------------- box


Physics::Body::Body(
    int physicsType, 
    float x, 
    float y, 
    float width, 
    float height, 
    int pointer, 
    bool isSensor, 
    float density, 
    float friction, 
    float restitution
) 
{
    this->width = width;
    this->height = height;
    this->shape = Body::Shape::BOX;

    Init(physicsType, x, y, pointer, isSensor, density, friction, restitution);
}


//----------------------------------- circle


Physics::Body::Body(
    int physicsType, 
    float x, 
    float y, 
    float radius, 
    int pointer, 
    bool isSensor, 
    float density, 
    float friction, 
    float restitution
) 
{
    this->radius = radius;
    this->shape = Body::Shape::CIRCLE;
    
    Init(physicsType, x, y, pointer, isSensor, density, friction, restitution);
}


//----------------------------------


void Physics::Body::Init(
    int physicsType, 
    float x, 
    float y, 
    int pointer, 
    bool isSensor, 
    float density, 
    float friction, 
    float restitution
)
{
    id = UUID::generate_uuid().c_str();
    type = physicsType;
    
    this->x = x;
    this->y = y;
    this->pointer = pointer;
    this->isSensor = isSensor;
    this->density = density; 
    this->friction = friction;
    this->restitution = restitution;
}


//----------------------------------


const bool Physics::Body::Exists() {
    const auto body = _GetBox2DBody(id);
    return body != nullptr;
}


//----------------------------------


b2FixtureDef MakeFixtureDef(bool isSensor, float density, float friction, float restitution) 
{
    b2FixtureDef def;

    def.isSensor = isSensor;
    def.density = density; 
    def.friction = friction; 
    def.restitution = restitution; 

    return def;
}


//----------------------------------


void Physics::Body::CreateFixture(float width, float height, bool isSensor, float density, float friction, float restitution) 
{
   const auto body = _GetBox2DBody(id);

    if (body) 
    {
        b2PolygonShape polygon;
        b2FixtureDef def = MakeFixtureDef(isSensor, density, friction, restitution);
        polygon.SetAsBox(width, height);
        def.shape = &polygon;

        body->CreateFixture(&def);
    }
}


//----------------------------------


void Physics::Body::CreateFixture(float radius, bool isSensor, float density, float friction, float restitution) 
{
   const auto body = _GetBox2DBody(id);

    if (body) 
    {
        b2CircleShape polygon;
        b2FixtureDef def = MakeFixtureDef(isSensor, density, friction, restitution);
        polygon.m_radius = radius;
        def.shape = &polygon;

        body->CreateFixture(&def);
    }
}


//----------------------------------


void Physics::Body::UpdateFixture(float width, float height, bool isSensor, float density, float friction, float restitution) 
{
    if (shape != Body::Shape::BOX)
        return;

    DestroyFixture();
    CreateFixture(width, height, isSensor, density, friction, restitution);
}


//----------------------------------


void Physics::Body::UpdateFixture(float radius, bool isSensor, float density, float friction, float restitution) 
{
    if (shape != Body::Shape::CIRCLE)
        return;

    DestroyFixture();
    CreateFixture(radius, isSensor, density, friction, restitution);
}


//----------------------------------


void Physics::Body::DestroyFixture() 
{ 
    const auto body = _GetBox2DBody(id);

    if (body) {

        b2Fixture* fixture = body->GetFixtureList(); 

        #if STANDALONE == 1
            fixture = body->GetFixtureList()->GetNext();
        #endif

        if (fixture)
            body->DestroyFixture(fixture); 
    }
}

//----------------------------------


const bool Physics::Body::CollidesWith(const std::shared_ptr<Physics::Body>& bodyB)
{
    const auto bA = _GetBox2DBody(id),
               bB = _GetBox2DBody(bodyB->id);

    if (!bA || !bB)
        return false;

    const auto aFix = bA->GetFixtureList(),
               bFix = bB->GetFixtureList();

    if (!aFix || !bFix)
        return false;

    return b2TestOverlap(aFix->GetAABB(0), bFix->GetAABB(0));
}



//----------------------------------


const bool Physics::Body::IsEnabled() {
    const auto body = _GetBox2DBody(id);
    if (body)
        return body->IsEnabled();
    return false;
}


//----------------------------------


const bool Physics::Body::IsSensor() {
    const auto body = _GetBox2DBody(id);
    if (body) 
        return !body->GetFixtureList() ? false : body->GetFixtureList()->IsSensor();

    return false;
}


//----------------------------------


const Math::Vector2 Physics::Body::GetPosition() {

    const auto body = _GetBox2DBody(id);

    if (body) {
        const b2Vec2& pos = body->GetPosition();
        return { pos.x, pos.y };
    }

    return { 0.0f, 0.0f };
}


//----------------------------------


const Math::Vector2 Physics::Body::GetLinearVelocity() {

    const auto body = _GetBox2DBody(id);

    if (body) {
        const b2Vec2& vel = body->GetLinearVelocity();
        return { vel.x, vel.y };
    }

    return { 0.0f, 0.0f };
}


//----------------------------------


const int Physics::Body::GetType() {
    const auto body = _GetBox2DBody(id);
    if (body)
        return body->GetType();

    return 0;
}


//----------------------------------


void Physics::Body::SetSensor(bool isSensor) 
{
    const auto body = _GetBox2DBody(id);
    
    if (body) 
    {
        const auto fixtureList = body->GetFixtureList();

        if (fixtureList)
            fixtureList->SetSensor(isSensor);
    }
}


//----------------------------------


void Physics::Body::SetEnabled(bool isEnabled) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->SetEnabled(isEnabled);
}


//----------------------------------


void Physics::Body::SetFixedRotation(bool isFixedRotation) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->SetFixedRotation(isFixedRotation);
}



//----------------------------------


void Physics::Body::SetLinearVelocity(float velocityX, float velocityY) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->SetLinearVelocity(b2Vec2(velocityX, velocityY));
}


//----------------------------------


void Physics::Body::ApplyLinearImpulse(float velocityX, float velocityY) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->ApplyLinearImpulse(b2Vec2(velocityX, velocityY), body->GetWorldCenter(), true);
}


//----------------------------------


void Physics::Body::SetTransform(float x, float y, float angle) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->SetTransform(b2Vec2(x, y), angle);
}


//----------------------------------


void Physics::Body::SetGravityScale(float gs) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->SetGravityScale(gs);
}




