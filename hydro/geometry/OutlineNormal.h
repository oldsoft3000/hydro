#ifndef OUTLINENORMAL_H
#define OUTLINENORMAL_H

#include <vector>
#include <memory>
#include <QVector3D>

enum OutlineNormalLinkType {
    LINK_NONE =             0x0,
    LINK_RAD_EQUAL =        0x1,
    LINK_RAD_BLOCK =        0x2,
    LINK_RAD_BRANCH =       0x4,
    LINK_TAN =              0x8,
};

class OutlineNormal;
typedef std::vector<OutlineNormal> OutlineNormals;

struct OutlineNormalLink {
    union {
        int         idx_normal;
        int         idx_normal_r;
    };

    union {
        int         idx_normal_pair;
        int         idx_normal_l;
    };

    OutlineNormalLinkType   type;
    int                     idx_row;

    OutlineNormalLink() {
        idx_normal_r = -1;
        idx_normal_l = -1;
        type = LINK_NONE;
        idx_row = -1;
    }

};

typedef std::list<OutlineNormalLink> OutlineNormalLinks;

enum class InnState {
    UNKNOWN,
    FIXED,
    DELETED,
    EDITED
};

/// Intersection type

typedef std::vector<std::pair<int, float>> NrmWeightsT;

struct InnT {
    QVector3D point;
    NrmWeightsT normals;
    float max_weight;
    float max_koef;
    InnState state;

    NrmWeightsT::iterator getInnWeightByNormal( int idx_normal ) {
        NrmWeightsT::iterator iresult = find_if( normals.begin(),
                        normals.end(),
                        [idx_normal]( const std::pair<int, float>& nrm_weight )->bool {
                            return idx_normal == nrm_weight.first; } );
        return iresult;
    }
};

typedef std::shared_ptr<InnT> InnPtr;
typedef std::list<std::pair<InnPtr, float>> InnWeightsT;

struct OutlineNormal {
    const std::vector<OutlineNormal>& outline_normals;

    QVector3D normal;
    QVector3D vertex_begin;
    QVector3D vertex_end;

    float kL;
    float kZ;

    int         idx;
    bool      is_closed;

    OutlineNormalLink       link_tan;

    OutlineNormalLinks      links_rad;
    OutlineNormalLinks      links_tan_r;
    OutlineNormalLinks      links_tan_l;

    std::vector<unsigned int> indicies;

    InnWeightsT inns;

    OutlineNormal( const std::vector<OutlineNormal>& outline_normals );
    bool isClosed() const;
    int getOppositeLink(bool cw = true) const;
    QVector3D calcVertex( unsigned int idx_row );
    int getIndex( unsigned int idx_row );
    QVector3D calcVertexZ( unsigned int idx_row );
    unsigned int calcNumSegments();
};


#endif // OUTLINENORMAL_H
