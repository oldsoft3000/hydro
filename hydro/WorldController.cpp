#include "WorldController.h"
#include "Atom.h"
#include "ElementUtils.h"
#include "Fluid.h"
#include "GlobalParams.h"
#include "FluidParams.h"
#include "Game.h"
#ifdef MEASURES
#include "Ticks.h"
#endif

const qint64 time_update_boundary = 30;

class DestructionListener : public b2DestructionListener
{
public:

    virtual void SayGoodbye(b2Joint* joint) {
        B2_NOT_USED(joint);
    }
    virtual void SayGoodbye(b2Fixture* fixture) {
        B2_NOT_USED(fixture);
    }
};


class ContactListner : public b2ContactListener
{
    WorldController& _controller;
public:
    ContactListner(WorldController& controller) : _controller(controller) {}

    virtual void EndContact(b2Contact* __attribute__((contact))) {
    }

    virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)  {
        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();

        b2Filter filterA = bodyA->GetFixtureList()->GetFilterData();
        b2Filter filterB = bodyB->GetFixtureList()->GetFilterData();

        if (filterA.categoryBits & CATEGORY_ATOM) {
            Atom* atomA = reinterpret_cast<Atom*>(bodyA->GetUserData());
            Atom* atomB = reinterpret_cast<Atom*>(bodyB->GetUserData());

            if (atomA->isCaptured() && atomB->isCaptured()) {
                if (filterB.categoryBits == CATEGORY_KERNEL) {
                    atomB = reinterpret_cast<Atom*>(bodyB->GetUserData());
                    atomB->linkAtom(atomA);

                } else if (filterB.categoryBits == CATEGORY_ATOM) {

                    if (atomA->isLinked() ||
                        atomB->isLinked()) {
                        atomB->linkAtom(atomA);
                    }
                }
            }
        }
    }
};


class ContactFilter : public b2ContactFilter
{
public:
    virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);
};


bool ContactFilter::ShouldCollide(b2Fixture* __attribute__((fixtureA)), b2Fixture* __attribute__((fixtureB))) {
    qDebug() << "ShouldCollide";
    return false;
}

WorldController::WorldController(const b2Vec2& gravity, float world_width, float world_height) : PhysicsController(gravity), _mouse_point(-1, -1)
{
    _contact_listner = std::make_shared<ContactListner>(*this);
    _contact_filter = std::make_shared<ContactFilter>();

    _world_width = world_width;
    _world_height = world_height;

    _mouse_joint = NULL;
    _active_kernel = NULL;

    _is_updates_enabled = true;
}

WorldController::~WorldController() {
    getWorld()->SetContactListener(NULL);
    getWorld()->SetContactFilter(NULL);
}

b2Vec2 _last_wc;

void WorldController::update() {
    if (!_is_updates_enabled) {
        return;
    }


    BaseClass::update();

    for (auto& fluid : _fluids) {
        fluid->update();

        b2Vec2 wc = fluid->getKernel()->getBody()->GetWorldCenter();
        if (wc.x != _last_wc.x || wc.y != _last_wc.y) {
            _last_wc = wc;
        }
    }

    if ( _active_kernel && _mouse_point.x > 0 ) {
        b2Vec2 p1 = _active_kernel->getBody()->GetFixtureList()->GetAABB(0).GetCenter();
        b2Vec2 p2 = _mouse_point;
        b2Vec2 vec_velocity = (p2 - p1);

        _active_kernel->getBody()->SetLinearVelocity( vec_velocity );
    }
}

void WorldController::updateElement(ElementBasePtr& atom) {
}

void WorldController::updateLinks(Atom* atom, Atom* kernel) {

    b2Vec2 vec_krn = kernel->getBody()->GetWorldCenter();
    b2Vec2 vec_atom = atom->getBody()->GetWorldCenter();

    float dist_krn_atom_sqr = (vec_krn - vec_atom).LengthSquared();

    if (dist_krn_atom_sqr > getMaxCaptureDistSqr()) {
        if (atom->getFluid() && atom->getFluid() == kernel->getFluid()) {
            atom->uncapture();
        }
        return;
    } else {
        if (kernel->getFluid()) {
            kernel->getFluid()->captureAtom(atom);
        }
    }

    AtomRawVector& links  = atom->getLinks();
    AtomRawVector::iterator ilnk;
    AtomRawVector unlink_queue;

    b2Vec2 force;
    b2Vec2 point;

    float r_atom = atom->getBody()->GetFixtureList()->GetShape()->m_radius;

    b2Vec2 vec_force_result(0, 0);

    for (ilnk = links.begin(); ilnk != links.end(); ++ilnk) {
        Atom* lnk = *ilnk;
        b2Vec2 vec_lnk = lnk->getBody()->GetWorldCenter();
        b2Vec2 vec_force = vec_lnk - vec_atom;
        float dist_lnk_sqr = vec_force.LengthSquared();
        float dist_krn_lnk_sqr = (vec_krn - vec_lnk).LengthSquared();
        float dist_atom_lnk_sqr = (vec_atom - vec_lnk).LengthSquared();
        float r_lnk = lnk->getBody()->GetFixtureList()->GetShape()->m_radius;
        float force_koef = 200 * pow(r_atom, 2) * pow(r_lnk, 2);

        if (dist_krn_lnk_sqr < dist_krn_atom_sqr) {
            vec_force = b2Vec2( force_koef * vec_force.x / (dist_lnk_sqr), force_koef * vec_force.y / (dist_lnk_sqr) );
            atom->getBody()->ApplyForce( vec_force, vec_lnk, false );
            vec_force_result += vec_force;
        }

        if (dist_atom_lnk_sqr > getMaxLinkDistSqr(atom, lnk)) {
            unlink_queue.push_back(lnk);
        }
    }

    for (ilnk = unlink_queue.begin(); ilnk != unlink_queue.end(); ++ilnk) {
        atom->unlinkAtom(*ilnk);
    }

    if (vec_force_result.LengthSquared() == 0) {
        atom->unlink();
    }

    force = vec_krn - vec_atom;

    const float force_koef = 700000;

    force = b2Vec2( force_koef * force.x / (dist_krn_atom_sqr), force_koef * force.y / (dist_krn_atom_sqr) );
}

AtomPtr WorldController::createAtom(const b2Vec2& pos, float32 radius, short categotyBits) {
    // body
    b2BodyDef bd;


    bd.position = pos;

    // shape
    b2CircleShape shape;
    shape.m_radius = radius;
    //b2PolygonShape shape;
    //shape.SetAsBox(radius/2.0f, radius/2.0f);
    // fixture
    b2FixtureDef fd;
    fd.shape = &shape;
    fd.friction = 0.0f; //трение
    fd.restitution = 0.0f; //упругость

    fd.filter.categoryBits = categotyBits;
    fd.filter.maskBits = MASK_ATOMS;

    switch (categotyBits) {
    case CATEGORY_KERNEL:
        //bd.type = b2_kinematicBody;
        bd.type = b2_dynamicBody;
        fd.density = 10;
        bd.linearDamping = DUMPING_LINEAR_KERNEL;
        break;
    case CATEGORY_ATOM:
        bd.type = b2_dynamicBody;
        fd.density = 0.1;
        bd.linearDamping = DUMPING_LINEAR_ATOM_0;
        break;
    case CATEGORY_ATOM_VIRTUAL:
        bd.type = b2_dynamicBody;
        fd.density = 0.1;
        bd.linearDamping = 0.0;
        break;
    default:
        break;
    }

    AtomPtr fluid = BaseClass::createElement<Atom>(bd, fd);

    return fluid;
}

ElementBasePtr WorldController::createRect(float32 x, float32 y, float32 w, float32 h, float32 angle, b2BodyType body_type) {
    ElementBasePtr atom = BaseClass::createRect(x, y, w, h, angle, body_type);
    atom->getBody()->GetFixtureList()->SetDensity(0.1);
    atom->getBody()->GetFixtureList()->SetFriction(0.3);
    return atom;
}

ElementBasePtr WorldController::createCircle(const b2Vec2& pos, float32 radius, b2BodyType body_type) {
    ElementBasePtr atom = BaseClass::createCircle(pos, radius, body_type);
    atom->getBody()->GetFixtureList()->SetDensity(1.0); //удельный вес
    atom->getBody()->GetFixtureList()->SetFriction(1.0); //трение
    atom->getBody()->GetFixtureList()->SetRestitution(0.6); //упругость
    return atom;
}

void WorldController::createBoundaries() {
    float wall_thickness = 2 / WORLD_SCALE;

    createRect(0.0f, _world_height - wall_thickness, _world_width, wall_thickness, 0, b2_staticBody);
    createRect(0.0f, 0.0f, wall_thickness, _world_height, 0, b2_staticBody);
    createRect(_world_width - wall_thickness, 0.0f, wall_thickness, _world_height, 0, b2_staticBody);

}

void WorldController::createObstacles() {
    //createRect(140.0f, 320.0f, 80.0f, 10.0f, 0.0f*b2_pi, b2_staticBody);
    //createRect(40.0f, 480.0f, 80.0f, 10.0f, 0.25f*b2_pi, b2_staticBody);
    //createRect(240.0f, 480.0f, 80.0f, 10.0f, -0.25f*b2_pi, b2_staticBody);
}

void WorldController::createFluid() {
    FluidPtr fluid = std::make_shared<Fluid>(shared_from_this(), b2Vec2(0.5 * getWorldWidth(), 0.5 * getWorldHeight()), KERNEL_RADIUS, ATOM_RADIUS);
    fluid->init();
    _fluids.push_back(fluid);
}

void WorldController::createMatter() {
    for ( int i = 0; i != 200; ++i ) {
        float dx = getWorldWidth() / 2 - qrand() % (int)getWorldWidth();
        float dy = qrand() % 20;
        ElementBasePtr atom_fluid = createAtom(b2Vec2(getWorldWidth() / 2 + dx, getWorldHeight() - dy - 200), 6.0f, CATEGORY_ATOM);
    }
}

void WorldController::mousePress(const b2Vec2& point) {

    b2Fixture* fixture = getFixtureAtPoint(point);
    if (fixture) {
        ElementBase* atom = reinterpret_cast<ElementBase*>(fixture->GetBody()->GetUserData());
        if ( atom ) {
            _active_kernel = atom;
        }
    }
}

void WorldController::mouseRelease(const b2Vec2& point) {
    _active_kernel = nullptr;

    _mouse_point = b2Vec2(-1, -1);
}

void WorldController::mouseMove(const b2Vec2& point) {
    if ( _active_kernel ) {
       _mouse_point = point;
    }
}

float WorldController::getMaxCaptureDistSqr() const {
    return 200 * 200;
}

float WorldController::getMaxLinkDistSqr(Atom* atom_1, Atom* atom_2) const {
    return pow(atom_1->getBody()->GetFixtureList()->GetShape()->m_radius * 1.5 +
               atom_2->getBody()->GetFixtureList()->GetShape()->m_radius * 1.5,
               2);
}


