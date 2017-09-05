#pragma once

#include <QtOpenGL>
#include <Box2D/Box2D.h>

class ElementBase
{
protected:
    ElementBase(b2World* world, const b2BodyDef& bd, const b2FixtureDef& fd);
    ElementBase(b2World* world, const b2BodyDef& bd, const std::vector<b2FixtureDef>& fds);
    ElementBase(b2World* world, const b2BodyDef& bd);

public:
    template<typename ElementType> static std::shared_ptr<ElementType> create(b2World* world, const b2BodyDef& bd, const b2FixtureDef& fd) {
        struct make_shared_enabler : public ElementType {
            make_shared_enabler(b2World* world, const b2BodyDef& bd, const b2FixtureDef& fd) : ElementType(world, bd, fd) {}
        };
        std::shared_ptr<ElementType> element = std::make_shared<make_shared_enabler>(world, bd, fd);
        return element;
    }
    template<typename ElementType> static std::shared_ptr<ElementType> create(b2World* world, const b2BodyDef& bd, const std::vector<b2FixtureDef>& fds) {
        struct make_shared_enabler : public ElementType {
            make_shared_enabler(b2World* world, const b2BodyDef& bd, const b2FixtureDef& fds) : ElementType(world, bd, fds) {}
        };
        std::shared_ptr<ElementType> element = std::make_shared<make_shared_enabler>(world, bd, fds);
        return element;
    }
    template<typename ElementType> static std::shared_ptr<ElementType> create(b2World* world, const b2BodyDef& bd) {
        struct make_shared_enabler : public ElementType {
            make_shared_enabler(b2World* world, const b2BodyDef& bd) : ElementType(world, bd) {}
        };
        std::shared_ptr<ElementType> element = std::make_shared<make_shared_enabler>(world, bd);
        return element;
    }  

    virtual ~ElementBase();
    b2Body* getBody() { return _body; }
    const b2Body* getBody() const { return _body; }
private:
    b2Body* _body;
};

typedef std::shared_ptr<ElementBase> ElementBasePtr;
typedef std::shared_ptr<const ElementBase> ElementBaseConstPtr;
