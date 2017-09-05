#include "Fluid.h"
#include "WorldController.h"
#include "Atom.h"
#include "GeometryUtils.h"
#include "ElementUtils.h"
#include "GlobalParams.h"
#include "FixturePainter.h"
#include "CommonUtils.h"
#include "Game.h"
#include "Interpolator.h"
#include "FluidParams.h"
#include "FluidGeometry.h"

#include <cmath>
#include <limits>

const int ID_TANGENCIAL_JOINT = 0x01;
const int ID_RADIAL_JOINT = 0x02;
const int ID_PRISMATIC_JOINT = 0x04;
const int ID_REVOLUTE_JOINT = 0x08;

const qint64 TRACE_UPDATE_INTERVAL = 500;

class BoundaryCallback : public b2RayCastCallback {
public:
    BoundaryCallback() {
        _min_fraction = 0;
        _result = 0;
    }

    virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& __attribute__((point)),
                                  const b2Vec2& __attribute__((normal)), float32 fraction) {

        Atom* atom = getAtom( fixture->GetBody() );
        if ( atom ) {\
            if (_min_fraction == 0 || fraction < _min_fraction) {
                _result = atom;
                _min_fraction = fraction;
            }

            return fraction;
        }
        return -1;
    }

    Atom* getResult() const { return _result; }
private:
    Atom*       _result;
    float32     _min_fraction;
};

Fluid::Fluid(WorldControllerWeakPtr world_controller,
                         const b2Vec2& pos,
                         float kernel_radius,
                         float atom_radius) :_world_controller(world_controller),
                                                _kernel_radius(kernel_radius),
                                                _atom_radius(atom_radius)
{

    auto conv_ca = [](const QVector2D& v) -> double { return v.length(); };
    _vcl_ca.setConvFunc(conv_ca);
    auto conv_freq = [](float v) -> float { return v; };
    _vcl_freq_rad.setConvFunc(conv_freq);
    _vcl_freq_tan.setConvFunc(conv_freq);
    _vcl_dmp_rad.setConvFunc(conv_freq);
    _vcl_dmp_lnr.setConvFunc(conv_freq);

    _kernel = nullptr;

    create(pos);
}

Fluid::~Fluid() {
    _captured_atoms.clear();
    _boundary_atoms.clear();
}

void Fluid::init() {
    _kernel->setFluid(shared_from_this());
}

void Fluid::joinDistance(b2Body* bodyA, b2Body* bodyB, float32 frequencyHz, float32 dampingRatio, int userData) {
    b2DistanceJointDef jointDef;

    jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldCenter(), bodyB->GetWorldCenter());
    jointDef.collideConnected = true;
    jointDef.frequencyHz = frequencyHz;
    jointDef.dampingRatio = dampingRatio;
    b2Joint* joint = bodyA->GetWorld()->CreateJoint( &jointDef );
    joint->SetUserData(reinterpret_cast<void*>(userData));
}

void Fluid::joinPrismatic(b2Body* bodyA, b2Body* bodyB, int userData) {
    b2PrismaticJointDef prismaticJointDef;
    prismaticJointDef.bodyA = bodyA;
    prismaticJointDef.bodyB = bodyB;

    b2Vec2 vec = bodyB->GetWorldCenter() - bodyA->GetWorldCenter();
    vec.Normalize();
    prismaticJointDef.localAxisA = vec;
    prismaticJointDef.lowerTranslation = PRISMATIC_LOWER_TRANSLATION;
    prismaticJointDef.upperTranslation = PRISMATIC_UPPER_TRANSLATION;
    prismaticJointDef.enableLimit = true;
    prismaticJointDef.collideConnected = true;

    b2Joint* joint = bodyA->GetWorld()->CreateJoint( &prismaticJointDef );
    joint->SetUserData(reinterpret_cast<void*>(userData));
}

void Fluid::joinRevolute(b2Body* bodyA, b2Body* bodyB, int userData) {
    b2RevoluteJointDef jointDef;

    jointDef.bodyA = bodyA;
    jointDef.bodyB = bodyB;
    //jointDef.localAnchorA.Set(_atom_radius/2, _atom_radius/2);
    //jointDef.localAnchorB.Set(_atom_radius/2, _atom_radius/2);

    jointDef.upperAngle = ANGLE_REVOLUTE;
    jointDef.lowerAngle = -ANGLE_REVOLUTE;
    jointDef.enableLimit = true;
    jointDef.collideConnected = false;

    b2Joint* joint = bodyA->GetWorld()->CreateJoint( &jointDef );
    joint->SetUserData(reinterpret_cast<void*>(userData));
}

void Fluid::captureAtom(Atom* atom) {
    if (atom->getFluid()) {
        if (atom->getFluid().get() != this) {
            atom->uncapture();
        } else {
            return;
        }
    }
    atom->setFluid(shared_from_this());
    _captured_atoms.push_back(atom);
}

void Fluid::uncaptureAtom(Atom* atom) {
    _captured_atoms.remove(atom);
    atom->setFluid(FluidPtr());
}

float Fluid::getMaxBoundaryDistSqr() const {
    WorldControllerPtr wc = _world_controller.lock();
    float r = wc->getMaxCaptureDistSqr();
    return r;
}

float Fluid::getOuterRadius() const {
    return _kernel_radius * 2.5;
}

float Fluid::getOuterRadiusSqr() const {
    float radius = getOuterRadius();

    return radius * radius;
}

void Fluid::create(const b2Vec2& pos) {
    WorldControllerPtr wc = _world_controller.lock();
    if (wc) {
        _kernel = wc->createAtom(pos, _kernel_radius, CATEGORY_KERNEL).get();

        float radius = getOuterRadius();
        float angle = 0;

        //int num_atoms = 2 * M_PI * radius / (_atom_radius * 4);
        int num_atoms = NUM_ATOMS;
        float delta_angle = 2 * M_PI / num_atoms;

        for (int iatom = 0; iatom != num_atoms; iatom++) {
            float x = pos.x + radius * cos(angle);
            float y = pos.y + radius * sin(angle);
            ElementBasePtr atom_1 = wc->createAtom(b2Vec2(x, y), _atom_radius, CATEGORY_ATOM);
            ElementBasePtr atom_2 = wc->createAtom(b2Vec2(x, y), _atom_radius, CATEGORY_ATOM_VIRTUAL);
            _atoms.push_back(std::make_pair(atom_1, atom_2));

            joinRevolute(atom_1->getBody(), atom_2->getBody(), ID_REVOLUTE_JOINT);

            angle += delta_angle;
        }

        for (Atoms::iterator j = _atoms.begin() + 1; j != _atoms.end(); ++j) {
            ElementBasePtr atom_1_1 = (j)->first;
            ElementBasePtr atom_0_2 = (j - 1)->second;
            ElementBasePtr atom_0_1 = (j - 1)->first;

            joinDistance(atom_0_1->getBody(), atom_1_1->getBody(), FREQ_TANGENCIAL, DUMPING_TANGENCIAL, ID_TANGENCIAL_JOINT);
            joinDistance(_kernel->getBody(), atom_1_1->getBody(), FREQ_RADIAL, DUMPING_RADIAL, ID_RADIAL_JOINT);
            joinPrismatic(atom_0_2->getBody(), atom_1_1->getBody(), ID_PRISMATIC_JOINT);
        }

        ElementBasePtr atom_1_1 = (_atoms.begin())->first;
        ElementBasePtr atom_0_2 = (_atoms.end() - 1)->second;
        ElementBasePtr atom_0_1 = (_atoms.end() - 1)->first;

        joinDistance(atom_0_1->getBody(), atom_1_1->getBody(), FREQ_TANGENCIAL, DUMPING_TANGENCIAL, ID_TANGENCIAL_JOINT);
        joinDistance(_kernel->getBody(), atom_1_1->getBody(), FREQ_RADIAL, DUMPING_RADIAL, ID_RADIAL_JOINT);
        joinPrismatic(atom_0_2->getBody(), atom_1_1->getBody(), ID_PRISMATIC_JOINT);
    }
}

void Fluid::update() {
    updateSoftening();
    updateTrace();
}


void Fluid::updateTrace() {
    b2Vec2 pc = _kernel->getBody()->GetWorldCenter();
    _ca.update(QVector2D(pc.x, pc.y));
}

void setDistanceJointParams(b2JointEdge* joint_list, int idJoint, b2Body* body_1, b2Body* body_2, float freq, float dumping) {
    while (joint_list) {
        b2Joint* joint = joint_list->joint;

        if (joint->GetUserData() == reinterpret_cast<void*>(idJoint)) {
            if (joint->GetType() == e_distanceJoint &&
                joint->GetBodyA() == body_1 &&
                joint->GetBodyB() == body_2) {
                b2DistanceJoint* distance_joint = static_cast<b2DistanceJoint*>(joint);
                distance_joint->SetFrequency(freq);
                distance_joint->SetDampingRatio(dumping);
            }
        }

        joint_list = joint_list->next;
    }
};

void setDistanceJointParams(b2JointEdge* joint_list, int idJoint, float freq, float dumping) {
    while (joint_list) {
        b2Joint* joint = joint_list->joint;

        if (joint->GetUserData() == reinterpret_cast<void*>(idJoint)) {
            if (joint->GetType() == e_distanceJoint) {
                b2DistanceJoint* distance_joint = static_cast<b2DistanceJoint*>(joint);
                distance_joint->SetFrequency(freq);
                distance_joint->SetDampingRatio(dumping);
            }
        }

        joint_list = joint_list->next;
    }
};

void setRevoluteJointParams(b2JointEdge* joint_list, int idJoint, float32 angle) {
    while (joint_list) {
        b2Joint* joint = joint_list->joint;

        if (joint->GetUserData() == reinterpret_cast<void*>(idJoint)) {
            if (joint->GetType() == e_revoluteJoint) {
                b2RevoluteJoint* revolute_joint = static_cast<b2RevoluteJoint*>(joint);
                revolute_joint->SetLimits(-angle, angle);
            }
        }

        joint_list = joint_list->next;
    }
};

void setPrismaticJointParams(b2JointEdge* joint_list, int idJoint, float32 lower_translation, float32 upper_translation ) {
    while (joint_list) {
        b2Joint* joint = joint_list->joint;

        if (joint->GetUserData() == reinterpret_cast<void*>(idJoint)) {
            if (joint->GetType() == e_prismaticJoint) {
                b2PrismaticJoint* prismatic_joint = static_cast<b2PrismaticJoint*>(joint);
                prismatic_joint->SetLimits(lower_translation, upper_translation);
            }
        }

        joint_list = joint_list->next;
    }
};

void Fluid::updateSoftening() {
    b2Vec2 vec_velocity_kernel = _kernel->getBody()->GetLinearVelocity();
    float angle_velocity_kernel = calcAngle(vec_velocity_kernel.x, vec_velocity_kernel.y);

    long long current_time = Game::instance().getTimerNano().elapsed();

    Atoms::iterator iatom_tail = _atoms.end();
    Atoms::iterator iatom_head = _atoms.end();
    Atoms::iterator ibound_head_0 = _atoms.end();
    Atoms::iterator ibound_head_1 = _atoms.end();
    Atoms::iterator ibound_tail_0_0 = _atoms.end();
    Atoms::iterator ibound_tail_0_1 = _atoms.end();
    Atoms::iterator ibound_tail_1_0 = _atoms.end();
    Atoms::iterator ibound_tail_1_1 = _atoms.end();
    Atoms::iterator ibound_head_0_0 = _atoms.end();
    Atoms::iterator ibound_head_0_1 = _atoms.end();

    float tail_length = 0;
    float min_angle = 2 * M_PI;

    int idx_atom = 0;

    int num_head_atoms = KOEF_NUM_HEAD_ATOMS * NUM_ATOMS;
    int num_tail_atoms = NUM_ATOMS - num_head_atoms;

    for (Atoms::iterator iatom = _atoms.begin(); iatom != _atoms.end(); ++iatom, ++idx_atom) {
        ElementBasePtr atom = iatom->first;
        b2Vec2 vec_atom = atom->getBody()->GetWorldCenter() - _kernel->getBody()->GetWorldCenter();
        float angle_atom = calcAngle(vec_atom.x, vec_atom.y);
        float angle = std::abs(angle_velocity_kernel - angle_atom);

        if (vec_atom.Length() > tail_length) {
            tail_length = vec_atom.Length();
            iatom_tail = iatom;
        }

        if (angle < min_angle) {
            min_angle = angle;
            iatom_head = iatom;
        }
    }

    if (num_tail_atoms % 2 == 0) {
        num_tail_atoms++;
        num_head_atoms--;
    }

    iatom_tail = advance_cycled(_atoms.begin(), _atoms.end(), iatom_head, NUM_ATOMS / 2);


    ibound_head_0 = advance_cycled(_atoms.begin(), _atoms.end(), iatom_head, -num_head_atoms / 2);
    ibound_head_1 = advance_cycled(_atoms.begin(), _atoms.end(), iatom_head, num_head_atoms - num_head_atoms / 2 - 1);

    ibound_tail_0_0 = advance_cycled(_atoms.begin(), _atoms.end(), ibound_head_1, 1);
    ibound_tail_0_1 = iatom_tail;
    ibound_tail_1_0 = advance_cycled(_atoms.begin(), _atoms.end(), iatom_tail, 1);
    ibound_tail_1_1 = ibound_head_0;
    ibound_head_0_0 = advance_cycled(_atoms.begin(), _atoms.end(), ibound_head_0, 1);
    ibound_head_0_1 = ibound_head_1;

    unsigned int distance_0 = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_0_0, ibound_tail_0_1);
    unsigned int distance_1 = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_1_0, ibound_tail_1_1);

    Interpolator intp_tan_1(0,
                             distance_0,
                             FREQ_TANGENCIAL_0,
                             FREQ_TANGENCIAL_1);
    Interpolator intp_tan_2(0,
                             distance_1,
                             FREQ_TANGENCIAL_0,
                             FREQ_TANGENCIAL_1);

    for (Atoms::iterator iatom = _atoms.begin(); iatom != _atoms.end(); ++iatom, ++idx_atom) {
        ElementBasePtr atom = iatom->first;

        double speed = 0;
        float freq_tan = 0;
        float freq_tan_intp = 0;

        if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_head_0_0, ibound_head_0_1)) {
            speed = 2;
            freq_tan = FREQ_TANGENCIAL;
        } else {
            speed = 100 * FREQ_TANGENCIAL_0 / FREQ_TANGENCIAL_1;
            if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_0_0, ibound_tail_0_1)) {
                unsigned int distance = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_0_0, iatom);
                freq_tan = intp_tan_1.calc(distance, QEasingCurve::OutExpo);
                //freq_tan = FREQ_TANGENCIAL_1;
                //qDebug() << "left distance " << distance << " " << idx_atom << " " << freq_tan;
            } else if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_1_0, ibound_tail_1_1)) {
                unsigned int distance = distance_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_1_1);
                freq_tan = intp_tan_2.calc(distance, QEasingCurve::OutExpo);
                //freq_tan = FREQ_TANGENCIAL_1;
                //qDebug() << "right distance " << distance << " " << idx_atom << " " << freq_tan;
            }
        }

        _vcl_freq_tan.calcValue(idx_atom,
                                   current_time,
                                   speed,
                                   FREQ_TANGENCIAL_0,
                                   freq_tan,
                                   freq_tan_intp);

        setDistanceJointParams(atom->getBody()->GetJointList(), ID_TANGENCIAL_JOINT, freq_tan, DUMPING_TANGENCIAL);
    }

    ibound_tail_0_0 = advance_cycled(_atoms.begin(), _atoms.end(), ibound_head_1, 1);
    ibound_tail_0_1 = iatom_tail;
    ibound_tail_1_0 = iatom_tail;
    ibound_tail_1_1 = advance_cycled(_atoms.begin(), _atoms.end(), ibound_head_0, -1);
    ibound_head_0_0 = ibound_head_0;
    ibound_head_0_1 = ibound_head_1;

    distance_0 = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_0_0, ibound_tail_0_1);
    distance_1 = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_1_0, ibound_tail_1_1);

    Interpolator intp_rad_1(0,
                             distance_0,
                             FREQ_RADIAL_0,
                             FREQ_RADIAL_1);
    Interpolator intp_rad_2(0,
                             distance_1,
                             FREQ_RADIAL_0,
                             FREQ_RADIAL_1);

    Interpolator intp_dmp_1(0,
                            distance_0,
                             DUMPING_LINEAR_ATOM_0,
                             DUMPING_LINEAR_ATOM_1);
    Interpolator intp_dmp_2(0,
                            distance_1,
                             DUMPING_LINEAR_ATOM_0,
                             DUMPING_LINEAR_ATOM_1);

    Interpolator intp_dmp_rad_1(0,
                             distance_0,
                             DUMPING_RADIAL_0,
                             DUMPING_RADIAL_1);
    Interpolator intp_dmp_rad_2(0,
                             distance_1,
                             DUMPING_RADIAL_0,
                             DUMPING_RADIAL_1);


    for (Atoms::iterator iatom = _atoms.begin(); iatom != _atoms.end(); ++iatom, ++idx_atom) {
        ElementBasePtr atom = iatom->first;

        b2Vec2 vec_velocity_atom = atom->getBody()->GetLinearVelocity();

        double speed_freq = 0;
        float freq_rad = 0;
        float freq_rad_intp = 0;
        //float dmp_rad = std::min(DUMPING_RADIAL / velocity_kernel, DUMPING_RADIAL);
        //float dmp_rad_intp = 0;
        float dmp_lnr = 0;
        //float dmp_lnr_intp = 0;

        float prismatic_upper_translation = PRISMATIC_UPPER_TRANSLATION;

        if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_head_0_0, ibound_head_0_1)) {
            speed_freq = 2;
            freq_rad = FREQ_RADIAL;
            //dmp_rad = DUMPING_RADIAL_0;
            dmp_lnr = DUMPING_LINEAR_ATOM_0;

            prismatic_upper_translation = KERNEL_RADIUS * 1.5;
        } else {
            speed_freq = 100 * FREQ_RADIAL_0 / FREQ_RADIAL_1;
            if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_0_0, ibound_tail_0_1)) {
                unsigned int distance = distance_cycled(_atoms.begin(), _atoms.end(), ibound_tail_0_0, iatom);
                freq_rad = intp_rad_1.calc(distance, QEasingCurve::OutExpo);
                dmp_lnr = intp_dmp_1.calc(distance, QEasingCurve::InExpo);
            } else if (in_bounds_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_1_0, ibound_tail_1_1)) {
                unsigned int distance = distance_cycled(_atoms.begin(), _atoms.end(), iatom, ibound_tail_1_1);
                freq_rad = intp_rad_2.calc(distance, QEasingCurve::OutExpo);
                dmp_lnr = intp_dmp_2.calc(distance, QEasingCurve::InExpo);
            }
        }

        _vcl_freq_rad.calcValue(idx_atom,
                                   current_time,
                                   speed_freq,
                                   FREQ_RADIAL_0,
                                   freq_rad,
                                   freq_rad_intp);


        setPrismaticJointParams(atom->getBody()->GetJointList(), ID_PRISMATIC_JOINT, PRISMATIC_LOWER_TRANSLATION, prismatic_upper_translation);

        iatom->first->getBody()->SetLinearDamping(dmp_lnr);
        iatom->second->getBody()->SetLinearDamping(dmp_lnr);

        float32 dot_product = b2Dot(vec_velocity_kernel, vec_velocity_atom);
        if (dot_product > 0) {
            setDistanceJointParams(atom->getBody()->GetJointList(), ID_RADIAL_JOINT, freq_rad, DUMPING_RADIAL_0);
        } else {
            setDistanceJointParams(atom->getBody()->GetJointList(), ID_RADIAL_JOINT, freq_rad, DUMPING_RADIAL_1);
        }

    }
}

std::vector<FluidVertex>& Fluid::getBoundaryVertecies() {
    //_boundary_vertecies_interpolated = _boundary_vertecies;

    _boundary_vertecies_interpolated.clear();
    for (auto atom : _atoms) {
        FluidVertex fv;

        fv.cached_radius = 0;
        b2Vec2 atom_center = atom.first->getBody()->GetWorldCenter();

        fv.vertex = QVector2D(atom_center.x, atom_center.y);
        fv.atom = static_cast<Atom*>(atom.first.get());

        _boundary_vertecies_interpolated.push_back(fv);
    }

    return _boundary_vertecies_interpolated;
}

float Fluid::getVertexSpeed() const {
    return 0.1;
}
