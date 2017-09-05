#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <list>
#include <vector>

class Fluid;
class Atom;

typedef std::shared_ptr<Atom> AtomPtr;
typedef std::weak_ptr<Atom> AtomWeakPtr;

typedef std::list<AtomPtr> AtomList;
typedef std::list<Atom*> AtomRawList;
typedef std::vector<Atom*> AtomRawVector;

typedef std::shared_ptr<Fluid> FluidPtr;
typedef std::weak_ptr<Fluid> FluidWeakPtr;
typedef std::shared_ptr<const Fluid> FluidConstPtr;
typedef std::vector<FluidPtr> Fluids;


#endif // TYPES_H
