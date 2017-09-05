#include "PhysicsController.h"


class QueryCallback : public b2QueryCallback
{
public:
    QueryCallback(const b2Vec2& point) {
        m_point = point;
        m_fixture = NULL;
    }

    bool ReportFixture(b2Fixture* fixture){
        bool inside = fixture->TestPoint(m_point);
        if (inside) {
            m_fixture = fixture;
            return false;
        }
        return true;
    }

    b2Vec2 m_point;
    b2Fixture* m_fixture;
};

PhysicsController::PhysicsController(const b2Vec2& gravity)
{
    _world = std::make_shared<b2World>(gravity);
    _ground = ElementBase::create<ElementBase>(_world.get(), b2BodyDef());
}

PhysicsController::~PhysicsController() {
    clearElements();
}

void PhysicsController::update() {
    _world->Step(1.0f/60.0f, 10, 1);
    //_world->Step(1.0f/60.0f, 100, 1);

    /*for (Elements::iterator i = _elements.begin(); i != _elements.end(); ++i) {
        updateElement(*i);
    }*/
}

void PhysicsController::updateElement(ElementBasePtr& element) {

}

void PhysicsController::clearElements() {
    _elements.clear();
}


b2Fixture* PhysicsController::getFixtureAtPoint(const b2Vec2& point) {
    b2AABB aabb;
    b2Vec2 d;
    d.Set(0.001f, 0.001f);
    aabb.lowerBound = point - d;
    aabb.upperBound = point + d;

    QueryCallback callback(point);
    _world->QueryAABB(&callback, aabb);

    return callback.m_fixture;
}

ElementBasePtr PhysicsController::createRect(float32 x, float32 y, float32 w, float32 h, float32 angle, b2BodyType body_type) {
    // body
    b2BodyDef bd;
    bd.type = body_type;
    bd.position = b2Vec2(x+w/2.0f, y+h/2.0f);
    bd.angle = angle * b2_pi;
    // shape
    b2PolygonShape shape;
    shape.SetAsBox(w/2.0f, h/2.0f);
    // fixture
    b2FixtureDef fd;
    fd.shape = &shape;

    return createElement<ElementBase>( bd, fd );
}

ElementBasePtr PhysicsController::createCircle(const b2Vec2& pos, float32 radius, b2BodyType body_type) {
    // body
    b2BodyDef bd;
    bd.type = body_type;
    bd.position = pos;
    // shape
    b2CircleShape shape;
    shape.m_radius = radius;
    // fixture
    b2FixtureDef fd;
    fd.shape = &shape;

    return createElement<ElementBase>( bd, fd );
}


