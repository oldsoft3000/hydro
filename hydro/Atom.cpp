#include "Atom.h"
#include "Fluid.h"

Atom::Atom(b2World* world,
            const b2BodyDef& bd,
            const b2FixtureDef& fd) : ElementBase(world, bd, fd), _is_boundary(false)
{ 
}

void Atom::uncapture() {
    if (getFluid()) {
        getFluid()->uncaptureAtom(this);

        unlink();
    }
}

AtomLinkVector::iterator findLink(AtomLinkVector::iterator begin,
                                     AtomLinkVector::iterator end,
                                     Atom* atom) {
    AtomLinkVector::iterator result = begin;
    for (; result != end; ++result) {
        if (result->atom == atom) {
            break;
        }
    }
    return result;
}

void Atom::linkAtom(Atom* atom) {
    if (atom == this) {
        return;
    }

    if (find(_links.begin(), _links.end(), atom) != _links.end()) {
        return;
    }
    _links.push_back(atom);

    if (find(atom->_links.begin(), atom->_links.end(), this) != atom->_links.end()) {
        return;
    }
    atom->_links.push_back(this);

}

void Atom::unlink() {
    for (auto link : _links) {
        unlinkAtom(link);
    }
}

void Atom::unlinkAtom(Atom* atom) {
    if (atom == this) {
        return;
    }

    AtomRawVector::iterator ilink = find(_links.begin(), _links.end(), atom);
    if (ilink == _links.end()) {
        return;
    }
    _links.erase(ilink);

    ilink = find(atom->_links.begin(), atom->_links.end(), this);
    if (ilink == atom->_links.end()) {
        return;
    }
    atom->_links.erase(ilink);
}

