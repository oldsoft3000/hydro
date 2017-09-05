#ifndef FLUIDGEOMETRYTESTS_H
#define FLUIDGEOMETRYTESTS_H

#include "FluidGeometry.h"

class FluidGeometryTests
{
public:
    FluidGeometryTests(FluidGeometry& fluid_geometry);

    void testTriangulate();
    void testPoly2tri();
    void testDelaunay();
    QVector4D getColor();
private:
    FluidGeometry&    _fg;
    int _count_color;
};

#endif // FLUIDGEOMETRYTESTS_H
