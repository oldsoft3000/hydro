#ifndef FLUIDGEOMETRY_H
#define FLUIDGEOMETRY_H

//#include <QOpenGLFunctions>
//#include <QOpenGLShaderProgram>
//#include <QOpenGLBuffer>
#include "GeometryEngine.h"
#include <QColor>
#include <memory>
#include <set>
#include <bitset>
#include <functional>
#include "SnippetArea.h"
#include "CommonUtils.h"
#include "GeometryUtils.h"

enum OutlineNormalLinkType {
    LINK_NONE =             0x0,
    LINK_RAD_EQUAL =        0x1,
    LINK_RAD_BLOCK =        0x2,
    LINK_RAD_BRANCH =       0x4,
    //LINK_TAN_MAIN =         0x8,
    //LINK_TAN_SECONDARY =    0x10,
};

extern const float width_cell;
extern const int area_num_x;
extern const int area_num_y;
//extern const unsigned int outline_stride;
//extern const float normal_length;
extern const unsigned int outline_stride;
extern const float normal_length;
extern const int num_active_rows;

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


    //TODO change GLushort
    std::vector<GLushort>               indicies;

    const std::vector<OutlineNormal>&   outline_normals;

    OutlineNormal( const std::vector<OutlineNormal>& outline_normals ) : outline_normals(outline_normals) {
    }

    bool isClosed() const {
        return is_closed;
    }

    int getOppositeLink(bool cw = true) const {
        if ( !links_rad.empty() ) {
            if ( links_rad.size() == 1 ) {
                return links_rad.back().idx_normal;
            } else {
                std::vector<OutlineNormalLink>::const_iterator ilink = links_rad.begin();
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

    QVector3D calcVertex( unsigned int idx_row ) {
       unsigned int num_segments = calcNumSegments();

       if ( isClosed() && idx_row >= num_segments ) {
           return vertex_end;
       } else {
           return vertex_begin + normal * idx_row;
       }
    }

    int getIndex( unsigned int idx_row ) {
        if ( indicies.empty() ) {
            return -1;
        }

        if ( idx_row >= indicies.size() ) {
            idx_row = indicies.size() - 1;
        }

        return indicies[idx_row];
    }

    QVector3D calcVertexZ( unsigned int idx_row ) {
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


    unsigned int calcNumSegments() {
        double r = ( vertex_end - vertex_begin ).length() / normal_length;
        double i;
        double f = modf( r, &i );

        return ( f > 0 ) ? i + 1 : i;
    }
};


class FluidGeometry : public GeometryEngine
{
    friend class FluidTriangles;
    friend class FluidGeometryTests;

    typedef std::pair<OutlineNormals::iterator, OutlineNormals::iterator> sector_t;
    typedef std::vector<sector_t> sectors_t;

public:
    FluidGeometry();
    virtual ~FluidGeometry() {
    }

    void drawFluid(QOpenGLShaderProgram *program,
                   const std::vector<QVector2D>& outline_points);

    void init(unsigned int outline_points_size);
    bool isInited() { return _size_line != 0; }
private:

    void generateGeometry(const std::vector<QVector2D>& outline_points);

    void updateAreaTangen();
    void updateAreaRadial();
    void initOutlineNormals(const std::vector<QVector2D>& outline_points);

    void growNormal(OutlineNormals::iterator inormal);
    unsigned int  growNormals();
    void drawNormals();
    void clearBuffers();

    std::vector<OutlineNormal>::iterator getRadialIntersection(std::vector<OutlineNormal>::iterator inormal,
                                                               const sectors_t& sectors,
                                                               QVector3D& nearest_intersection,
                                                               float& min_length_sqr);

    OutlineNormals::iterator getOutlineNormal(unsigned int idx) {
        if (idx < _outline_normals.size()) {
            return _outline_normals.begin() + idx;
        } else {
            return _outline_normals.end();
        }
    }

    void linkNormalsRadial(OutlineNormals::iterator inormal_0,
                           OutlineNormals::iterator inormal_1,
                           QVector3D& intersection);

    void linkNormalsTangen(OutlineNormals::iterator inormal_0,
                           OutlineNormals::iterator inormal_1,
                           OutlineNormals::iterator inormal_2,
                           QVector3D& intersection);

    void addIndex(GLushort index, const QColor& color);
    void addVertex(const QVector3D& vertex);

    void drawLine(const QVector3D& vertex_0, const QVector3D& vertex_1, const QColor& color);


    float calcZ( OutlineNormals::iterator inormal );

    void fillVertices();
    void fillZ();
private:
    unsigned int    _size_line;


    SnippetArea                 _area;
    unsigned int                _frame_counter;

    std::vector<OutlineNormal>  _outline_normals;

    vertex_data_t              _vertex_data;
    index_data_t               _index_data;

    vertex_data_t              _vertex_data_lines;
    vertex_data_t              _vertex_data_triangles;

    std::vector<std::vector<unsigned int>>  _buf_idxs_rad;
    std::vector<unsigned int>               _buf_idxs_tan;


    unsigned int _idx_max_row;
};

typedef std::shared_ptr<FluidGeometry> FluidGeometryPtr;


#endif // FLUIDGEOMETRY_H
