#include <set>

#include "../../vendors/box2d/include/box2d/box2d.h"
#include "../../vendors/UUID.hpp"

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/physics.h"

typedef struct Box2D_Body {
    b2BodyDef def;
    b2Body* self;
    b2Fixture* fixture;
    b2FixtureDef fixtureDef;
};


static std::set<std::pair<const std::string, b2Body*>> _bodiesToRemove;
static std::map<const std::string, std::pair<const std::shared_ptr<Physics::Body>, b2Body*>> _active_bodies;
static b2World* _world;
static int _bodyCount = 0;


//---------------------------------- return underlying box2d body pointer to be used internally


static b2Body* _GetBox2DBody(const std::string& id) {
    auto it = _active_bodies.find(id);

    if (it != _active_bodies.end())
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


//------------------------------------


std::shared_ptr<Physics::Body> Physics::CreateBody(int type, int shape, float x, float y, float width, float height, bool isSensor, int pointer, float density, float friction, float restitution)
{
    Box2D_Body b2d_body;
    b2BodyType bodyType;

    if (type == Body::Type::STATIC)
        bodyType = b2_staticBody;

    else if (type == Body::Type::KINEMATIC)
        bodyType = b2_kinematicBody;

    else if (type == Body::Type::DYNAMIC)
        bodyType = b2_dynamicBody;

    else {
        LOG("Physics: invalid body type argument, defaulting to static.");
        bodyType = b2_staticBody;
    }

    b2d_body.def.type = bodyType;
    b2d_body.def.position.Set(x, y);

    b2d_body.def.userData.pointer = pointer <= -1 ? pointer : _bodyCount;
    b2d_body.self = _world->CreateBody(&b2d_body.def);

    b2CircleShape circle;
    b2PolygonShape box;

    if (shape == Body::Shape::CIRCLE) {
	    circle.m_radius = 0.3f;
        b2d_body.fixtureDef.shape = &circle;
    }

    if (shape == Body::Shape::BOX) {
        box.SetAsBox(width, height);
        b2d_body.fixtureDef.shape = &box;
    }

    else {
        box.SetAsBox(10, 10);
        b2d_body.fixtureDef.shape = &box;
    }

    b2d_body.fixtureDef.density = density;
    b2d_body.fixtureDef.friction = friction;
    b2d_body.fixtureDef.restitution = restitution;
    b2d_body.fixtureDef.isSensor = isSensor;

    b2d_body.self->CreateFixture(&b2d_body.fixtureDef);

    const auto body = std::make_shared<Body>(pointer <= -1 ? pointer : _bodyCount, isSensor, density, friction, restitution);

    _active_bodies.insert({ body->id, { body, b2d_body.self } });
    
    if (pointer > -1)
        _bodyCount++;

    return body;
}


//-----------------------------


void Physics::DestroyBody(const std::shared_ptr<Body> body) 
{
    const auto b2d_body = _GetBox2DBody(body->id);
    
    if (b2d_body) {
        _bodiesToRemove.insert({ body->id, b2d_body }); 
        _bodyCount--; 
    }
}


//------------------------------


void Physics::ClearBodies()
{
    if (_active_bodies.size())
        for (const auto& body : _active_bodies)
            _bodiesToRemove.insert({ body.first, body.second.second });

    _active_bodies.clear();

    Cleanup();

    LOG("Physics: bodies cleared.");
}


//-------------------------------


void Physics::Cleanup() 
{
    for (auto it = _bodiesToRemove.begin(); it != _bodiesToRemove.end(); ++it)
    {
        auto body = *it;
        auto b_it = std::find_if(_active_bodies.begin(), _active_bodies.end(), [body](const auto& b) { return b.first == body.first; });

        if (b_it != _active_bodies.end()) {
            b_it = _active_bodies.erase(b_it);
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


/* Body */


Physics::Body::Body(int pointer, bool isSensor, float density, float friction, float restitution) {
    id = UUID::generate_uuid().c_str();
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
        return body->GetFixtureList()->IsSensor();
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


void Physics::Body::SetSensor(bool isSensor) {
    const auto body = _GetBox2DBody(id);
    if (body)
        body->GetFixtureList()->SetSensor(isSensor);
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


//----------------------------------


void Physics::Body::DestroyFixture() { 
    const auto body = _GetBox2DBody(id);
    if (body)
        body->DestroyFixture(body->GetFixtureList());
}



//----------------------------------


void Physics::Body::CreateFixture(void* fixtureDef) {
   const auto body = _GetBox2DBody(id);
    if (body) {
        const auto def = static_cast<b2FixtureDef*>(fixtureDef);
        body->CreateFixture(def);
    }
}