#ifndef OUTLINENORMAL_H
#define OUTLINENORMAL_H

#include <vector>
#include <QVector3D>

enum OutlineNormalLinkType {
    LINK_NONE =             0x0,
    LINK_RAD_EQUAL =        0x1,
    LINK_RAD_BLOCK =        0x2,
    LINK_RAD_BRANCH =       0x4,
    //LINK_TAN_MAIN =         0x8,
    //LINK_TAN_SECONDARY =    0x10,
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

    OutlineNormalLink() {
        idx_normal_r = -1;
        idx_normal_l = -1;
        type = LINK_NONE;
    }

};

typedef std::vector<OutlineNormalLink> OutlineNormalLinks;

struct OutlineNormal {
    QVector3D normal;
    QVector3D vertex_begin;
    QVector3D vertex_end;

    float kL;
    float kZ;

    int idx;
    bool is_closed;

    OutlineNormalLink                   link_tan;

    std::vector<OutlineNormalLink>      links_rad;
    std::vector<OutlineNormalLink>      links_tan_r;
    std::vector<OutlineNormalLink>      links_tan_l;

    std::vector<unsigned int>           indicies;
    const std::vector<OutlineNormal>&   outline_normals;

    OutlineNormal( const std::vector<OutlineNormal>& outline_normals );
    bool isClosed() const;
    int getOppositeLink(bool cw = true) const;
    QVector3D calcVertex( unsigned int idx_row );
    int getIndex( unsigned int idx_row );
    QVector3D calcVertexZ( unsigned int idx_row );
    unsigned int calcNumSegments();
};


#endif // OUTLINENORMAL_H
