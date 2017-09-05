#pragma once

#include <PhysicsController.h>
#include <memory>
#include "Types.h"

class ContactListner;
typedef std::shared_ptr<ContactListner> ContactListnerPtr;
class ContactFilter;
typedef std::shared_ptr<ContactFilter> ContactFilterPtr;

//extern const short group_fluid_kernel;
//extern const short group_fluid_atom;

class WorldController : public std::enable_shared_from_this<WorldController>, public PhysicsController
{
    friend class ContactListner;

    typedef PhysicsController BaseClass;
public:
    WorldController(const b2Vec2& gravity, float world_width, float world_height);
    ~WorldController();

    ElementBasePtr createRect(float32 x, float32 y, float32 w, float32 h, float32 angle, b2BodyType body_type);
    ElementBasePtr createCircle(const b2Vec2& pos, float32 radius, b2BodyType body_type);
    AtomPtr createAtom(const b2Vec2& pos, float32 radius, short categotyBits);

    void mousePress(const b2Vec2& point);
    void mouseRelease(const b2Vec2& point);
    void mouseMove(const b2Vec2& point);

    void createObstacles();
    void createBoundaries();
    void createFluid();
    void createMatter();

    virtual void update();

    float getWorldWidth() const { return _world_width; }
    float getWorldHeight() const { return _world_height; }
    float getMaxCaptureDistSqr() const;
    float getMaxLinkDistSqr(Atom* atom_1, Atom* atom_2) const;

    Fluids& getFluids() { return _fluids; }

    void enableUpdates(bool value) { _is_updates_enabled = value; }
    bool isUpdatesEnabled() const { return _is_updates_enabled; }
private:
    void updateLinks(Atom* atom, Atom* kernel);
    virtual void updateElement(ElementBasePtr& atom);
private:
    float     _world_width;
    float     _world_height;

    ContactListnerPtr       _contact_listner;
    ContactFilterPtr        _contact_filter;
    Fluids                  _fluids;
    b2MouseJoint*           _mouse_joint;
    ElementBase*            _active_kernel;
    b2Vec2                  _mouse_point;
    bool                    _is_updates_enabled;
};


typedef std::shared_ptr<WorldController> WorldControllerPtr;
typedef std::shared_ptr<const WorldController> WorldControllerConstPtr;
typedef std::weak_ptr<WorldController> WorldControllerWeakPtr;
