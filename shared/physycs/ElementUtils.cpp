#include "ElementUtils.h"
#include "GlobalParams.h"

Atom* getAtom( const b2Body* body ) {
    Atom* atom = NULL;
    b2Filter filter = body->GetFixtureList()->GetFilterData();
    if (filter.maskBits == MASK_ATOMS) {
        atom = reinterpret_cast<Atom*>(body->GetUserData());
    }
    return atom;
}

