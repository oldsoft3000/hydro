#pragma once

#include <Box2D/Box2D.h>
#include <QtOpenGL>
#include "ObjectBase.h"
#include "Atom.h"
#include "MathUtils.h"
#include "LinearSpeedInterpolator.h"
#include <list>
#include <queue>

class WorldController;
class BoundariesCallback;
typedef std::weak_ptr<WorldController> WorldControllerWeakPtr;
typedef std::vector<std::pair<ElementBasePtr, ElementBasePtr>> Atoms;
class FluidGeometry;
typedef std::shared_ptr<FluidGeometry> FluidGeometryPtr;

struct FluidVertex {
    float           cached_radius;
    QVector2D       vertex;
    Atom*           atom;
};

class Fluid : public std::enable_shared_from_this<Fluid>, public ObjectBase
{
    friend class BoundariesCallback;
public:
    Fluid(WorldControllerWeakPtr world_controller,
                const b2Vec2& pos,
                float kernel_radius,
                float atom_radius);
    ~Fluid();
    void init();
    void captureAtom(Atom* atom);
    void update();

    void updateSoftening();
    void updateAtomAngles();
    void updateStability();
    void updateTrace();
    void updateCentrifugalForce();

    void uncaptureAtom(Atom* atom);
    void updateBoundaries();

    Atom* getKernel() { return _kernel; }

    std::vector<FluidVertex>& getBoundaryVertecies();

    bool calcCentripetalAcceleration(QVector2D& acceleration, QVector2D& center);

    float getOuterRadius() const;
    float getOuterRadiusSqr() const;
    FluidGeometryPtr getGeometry() { return _geometry; }
    void setGeometry(FluidGeometryPtr geometry) { _geometry = geometry; }

private:
    void create(const b2Vec2& pos);
    void joinDistance(b2Body* bodyA, b2Body* bodyB, float32 frequencyHz, float32 dampingRatio, int userData);
    void joinPrismatic(b2Body* bodyA, b2Body* bodyB, int userData);
    void joinRevolute(b2Body* bodyA, b2Body* bodyB, int userData);
    float getMaxBoundaryDistSqr() const;

    float getVertexSpeed() const;
private:
    WorldControllerWeakPtr      _world_controller;
    Atom*                       _kernel;
    AtomRawList                 _captured_atoms;
    AtomRawList                 _boundary_atoms;

    std::vector<FluidVertex>    _boundary_vertecies;
    std::vector<FluidVertex>    _boundary_vertecies_interpolated;

    CentripetalAcceleration     _ca;
    LinearSpeedInterpolator<QVector2D>   _vcl_ca;
    LinearSpeedInterpolator<float>       _vcl_freq_rad;
    LinearSpeedInterpolator<float>       _vcl_freq_tan;
    LinearSpeedInterpolator<float>       _vcl_dmp_rad;
    LinearSpeedInterpolator<float>       _vcl_dmp_lnr;

    Atoms                       _atoms;

    float                       _kernel_radius;
    float                       _atom_radius;

    FluidGeometryPtr                    _geometry;
};

typedef std::shared_ptr<Fluid> FluidPtr;
typedef std::shared_ptr<const Fluid> FluidConstPtr;
