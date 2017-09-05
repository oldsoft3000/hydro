#ifndef FLUIDGEOMETRYUTILS_H
#define FLUIDGEOMETRYUTILS_H

#include "FluidGeometry.h"

class FluidGeometryUtils
{
public:
    FluidGeometryUtils();

    static bool isSectorsSplitted(OutlineNormals& normals,
                                  OutlineNormals::iterator inormal_0,
                                  OutlineNormals::iterator inormal_1,
                                  OutlineNormals::iterator ispliter_0,
                                  OutlineNormals::iterator ispliter_1);

    static bool isSectorsSplitted(OutlineNormals& normals,
                                  OutlineNormals::iterator inormal_0,
                                  OutlineNormals::iterator inormal_1,
                                  OutlineNormals::iterator ispliter);

    static bool isNormalInSector(OutlineNormals& normals,
                          OutlineNormals::iterator inormal,
                          OutlineNormals::iterator isector_0,
                          OutlineNormals::iterator isector_1);
};

#endif // FLUIDGEOMETRYUTILS_H
