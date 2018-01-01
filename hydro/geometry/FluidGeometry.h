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
#include "OutlineNormal.h"

extern const float width_cell;
extern const int area_num_x;
extern const int area_num_y;
//extern const unsigned int outline_stride;
//extern const float normal_length;
extern const unsigned int outline_stride;
extern const float normal_length;
extern const int num_active_rows;

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

    int getIdxVertex( int idx_normal, int idx_row ) const;
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
