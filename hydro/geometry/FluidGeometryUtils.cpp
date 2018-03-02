#include "FluidGeometryUtils.h"
#include "CommonUtils.h"

FluidGeometryUtils::FluidGeometryUtils()
{

}


bool FluidGeometryUtils::isSectorsSplitted(OutlineNormals& normals,
                                       OutlineNormals::iterator inormal_0,
                                       OutlineNormals::iterator inormal_1,
                                       OutlineNormals::iterator ispliter_0,
                                       OutlineNormals::iterator ispliter_1) {

    int splitter_distance = distance_cycled(normals.begin(), normals.end(), ispliter_0, ispliter_1);
    int distance_0 = distance_cycled(normals.begin(), normals.end(), ispliter_0, inormal_0);
    int distance_1 = distance_cycled(normals.begin(), normals.end(), ispliter_0, inormal_1);

    if ( inormal_0 == ispliter_0 ||
         inormal_0 == ispliter_1 ||
         inormal_1 == ispliter_0 ||
         inormal_1 == ispliter_1 ) {
        return false;
    }

    if ( ( distance_0 > splitter_distance ) == ( distance_1 > splitter_distance ) ) {
        return false;
    } else {
        return true;
    }
}

bool FluidGeometryUtils::isSectorsSplitted(OutlineNormals& normals,
                                       OutlineNormals::iterator inormal_0,
                                       OutlineNormals::iterator inormal_1,
                                       OutlineNormals::iterator ispliter) {
    if ( !ispliter->links_rad.empty() ) {
        OutlineNormalLinks::iterator ilink = ispliter->links_rad.begin();
        for (; ilink != ispliter->links_rad.end(); ++ilink ) {
            if ( isSectorsSplitted( normals, inormal_0, inormal_1, ispliter, normals.begin() + ilink->idx_normal ) ) {
                return true;
            }
        }
    }

    if ( ispliter->link_tan.idx_normal_l != -1 ) {
        if ( isSectorsSplitted( normals, inormal_0, inormal_1, ispliter, normals.begin() + ispliter->link_tan.idx_normal_l ) ) {
            return true;
        }
    }

    if ( ispliter->link_tan.idx_normal_r != -1 ) {
        if ( isSectorsSplitted( normals, inormal_0, inormal_1, ispliter, normals.begin() + ispliter->link_tan.idx_normal_r ) ) {
            return true;
        }
    }

    return false;
}

bool FluidGeometryUtils::isNormalInSector(OutlineNormals& normals,
                                      OutlineNormals::iterator inormal,
                                      OutlineNormals::iterator isector_0,
                                      OutlineNormals::iterator isector_1) {


    int distance_normal = distance_cycled(normals.begin(), normals.end(), isector_0, inormal);
    int distance_sector = distance_cycled(normals.begin(), normals.end(), isector_0, isector_1);

    if ( distance_normal <= distance_sector ) {
        return true;
    } else {
        return false;
    }
}
