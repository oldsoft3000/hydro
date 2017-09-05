#pragma once

#include <Box2D/Box2D.h>
#include <QtOpenGL>
#include <QVector>
#include <functional>

#include "ElementBase.h"

typedef std::vector<ElementBasePtr> Elements;
typedef std::shared_ptr<b2World>    WorldPtr;

typedef std::function<void(ElementBasePtr)> ElementCallback;

class PhysicsController
{
public:
    PhysicsController( const b2Vec2& gravity );
    ~PhysicsController();

    template<typename ElementType> std::shared_ptr<ElementType> createElement( const b2BodyDef& bd, const b2FixtureDef& fd ) {
        std::shared_ptr<ElementType> element = ElementBase::create<ElementType>( _world.get(), bd, fd );
        _elements.push_back( element );
        return element;
    }
    template<typename ElementType> std::shared_ptr<ElementType> createElement( const b2BodyDef& bd, const std::vector<b2FixtureDef>& fds ) {
        std::shared_ptr<ElementType> element = ElementBase::create<ElementType>( _world.get(), bd, fds );
        _elements.push_back( element );
        return element;
    }
    ElementBasePtr createRect(float32 x, float32 y, float32 w, float32 h, float32 angle, b2BodyType body_type);
    ElementBasePtr createCircle(const b2Vec2& pos, float32 radius, b2BodyType body_type);

    virtual void update();
    virtual void updateElement(ElementBasePtr& element);

    void clearElements();
    b2Fixture* getFixtureAtPoint(const b2Vec2& point);
    WorldPtr getWorld() { return _world; }
    ElementBasePtr& getGround() { return _ground; }
    Elements& getElements() { return _elements; }
private:
    WorldPtr        _world;
    ElementBasePtr  _ground;
    Elements        _elements;
};

typedef std::shared_ptr<PhysicsController> PhysicsControllerPtr;
typedef std::shared_ptr<const PhysicsController> PhysicsControllerConstPtr;
