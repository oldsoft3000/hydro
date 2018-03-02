#include "OutlineNormal.h"
#include "CommonUtils.h"
#include "FluidGeometry.h"

OutlineNormal::OutlineNormal( const std::vector<OutlineNormal>& outline_normals ) : outline_normals(outline_normals) {
}

bool OutlineNormal::isClosed() const {
    return is_closed;
}

int OutlineNormal::getOppositeLink( bool cw ) const {
    if ( !links_rad.empty() ) {
        if ( links_rad.size() == 1 ) {
            return links_rad.back().idx_normal;
        } else {
            OutlineNormalLinks::const_iterator ilink = links_rad.begin();
            int idx_result = ilink->idx_normal;
            unsigned int max_distance = 0;
            for (; ilink != links_rad.end(); ++ilink ) {
                unsigned int distance = distance_cycled(outline_normals.begin(),
                                                        outline_normals.end(),
                                                        outline_normals.begin() + idx,
                                                        outline_normals.begin() + ilink->idx_normal,
                                                        cw);
                if (distance > max_distance) {
                    idx_result = ilink->idx_normal;
                    max_distance = distance;
                }
            }
            return idx_result;
        }
    } else if ( cw && link_tan.idx_normal_l != -1 ) {
        return link_tan.idx_normal_l;
    } else if ( !cw && link_tan.idx_normal_r != -1 ) {
        return link_tan.idx_normal_r;
    } else {
        Q_ASSERT(false);
        return -1;
    }
}

QVector3D OutlineNormal::calcVertex( unsigned int idx_row ) {
   unsigned int num_segments = calcNumSegments();

   if ( isClosed() && idx_row >= num_segments ) {
       return vertex_end;
   } else {
       return vertex_begin + normal * idx_row;
   }
}

int OutlineNormal::getIndex( unsigned int idx_row ) {
    if ( indicies.empty() ) {
        return -1;
    }

    if ( idx_row >= indicies.size() ) {
        idx_row = indicies.size() - 1;
    }

    return indicies[idx_row];
}

QVector3D OutlineNormal::calcVertexZ( unsigned int idx_row ) {
    float Z = 0;
    float x = 0;
    QVector3D result;

    unsigned int num_segments = calcNumSegments();

    if ( isClosed() && idx_row >= num_segments ) {
        x = kL;
        result = vertex_end;
    } else {
        x = normal_length * idx_row;
        result = vertex_begin + normal * idx_row;
    }

    if ( kL ) {
        float b = ( kZ + kL * kL ) / kL;
        Z = - ( x * x ) + b * x;
        result.setZ( Z );
    }

    return result;
}


unsigned int OutlineNormal::calcNumSegments() {
    double r = ( vertex_end - vertex_begin ).length() / normal_length;
    double i;
    double f = modf( r, &i );

    return ( f > 0 ) ? i + 1 : i;
}
