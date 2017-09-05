#pragma once

#include <ElementBase.h>
#include <memory>
#include "Types.h"

struct AtomLink {
    Atom*   atom;
    float   dist_krn;

    AtomLink(Atom* atom_) {
        atom = atom_;
        dist_krn = 0;
    }

    bool operator == (const AtomLink& other) {
        return this->atom == other.atom;
    }
};

typedef std::vector<AtomLink> AtomLinkVector;


class Atom : public ElementBase
{
    friend class Fluid;
protected:
    Atom( b2World* world, const b2BodyDef& bd, const b2FixtureDef& fd);
public:
     FluidPtr getFluid() const { return  _fluid.lock(); }
     void uncapture();
     bool isCaptured() const { return _fluid.lock() != nullptr; }
     bool isBoundary() const { return _is_boundary; }
     bool isLinked() const { return !_links.empty(); }

     void linkAtom(Atom* atom);
     void unlinkAtom(Atom*atom);
     void unlinkAtomRecursive(Atom* atom);
     void unlink();

     AtomRawVector& getLinks() { return _links; }
     float getDistKrn() const { return _dist_krn; }

     const QColor& getColor() const { return _color; }
     void setColor(const QColor& color) { _color = color; }

private:
     void setFluid(FluidPtr fluid) { _fluid = fluid; }
     void setBoundary(bool is_boundary) { _is_boundary = is_boundary; }
     //void setLinked(bool is_linked) { _is_linked = is_linked; }
     void setDistKrn(float dist) { _dist_krn = dist; }
private:
     FluidWeakPtr           _fluid;
     bool                   _is_boundary;
     AtomRawVector          _links;
     float                  _dist_krn;
     QColor                 _color;
};


