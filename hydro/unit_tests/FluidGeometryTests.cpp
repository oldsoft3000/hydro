#include "FluidGeometryTests.h"
#include "Triangulate.h"
#include <vector>
#include <poly2tri.h>
#include "vector2.h"
#include "triangle.h"
#include "delaunay.h"

FluidGeometryTests::FluidGeometryTests(FluidGeometry& fluid_geometry) : _fg(fluid_geometry) {
    _count_color = 0;
}

void FluidGeometryTests::testTriangulate() {

    std::vector<QVector3D> vectors;

    /*vectors.push_back(QVector3D(2.5, 1.6, 0.0));
    vectors.push_back(QVector3D(2.8, 1.6, 0.0));
    vectors.push_back(QVector3D(2.8, 1.9, 0.0));
    vectors.push_back(QVector3D(2.65, 1.75, 0.0));
    vectors.push_back(QVector3D(2.5, 1.9, 0.0));
    vectors.push_back(QVector3D(2.5, 1.6, 0.0));
    vectors.push_back(QVector3D(2.5, 1.6, 0.0));*/

    vectors.push_back(QVector3D(2.5, 1.6, 0.0));
    vectors.push_back(QVector3D(2.6, 1.6, 0.0));
    vectors.push_back(QVector3D(2.6, 1.6, 0.0));
    vectors.push_back(QVector3D(2.7, 1.6, 0.0));
    vectors.push_back(QVector3D(2.6, 1.7, 0.0));
    vectors.push_back(QVector3D(2.5, 1.6, 0.0));

    /*for (std::vector<QVector3D>::iterator i = vectors.begin(); i != vectors.end(); i++) {
        _fg.addVertex(*i);
    }*/

    /*for (unsigned int i = 0; i != vectors.size(); i++) {
        _fg.addIndex(_fg._buf_vertex.size() - ( -i + vectors.size() ), QColor(255, 0, 0, 255));
    }*/

    std::vector<GLushort> result;

    //Triangulate::Process(_fg._buf_vertex, result);

    /*for (std::vector<unsigned int>::iterator i = result.begin(); i != result.end(); i++) {
        Vector2d v2d = contour[*i];
        _fg.addVertex(QVector3D(v2d.GetX(), v2d.GetY(), 0));
    }

    for (unsigned int i = 0; i != result.size(); i++) {
        _fg.addIndex(_fg._buf_vertex.size() - ( -i + result.size() ), QColor(255, 0, 0, 255));
    }*/
}


void FluidGeometryTests::testPoly2tri() {

    /*std::vector<p2t::Point*> polyline;
    std::vector<p2t::Triangle*> triangles;
    std::list<p2t::Triangle*> map;

    polyline.push_back(new p2t::Point(2.3, 1.5));
    polyline.push_back(new p2t::Point(2.7, 1.5));
    polyline.push_back(new p2t::Point(2.7, 1.9));
    polyline.push_back(new p2t::Point(2.6, 1.7));
    polyline.push_back(new p2t::Point(2.65, 1.55));
    polyline.push_back(new p2t::Point(2.3, 1.9));

    p2t::CDT* cdt = new p2t::CDT(polyline);

    cdt->Triangulate();
    triangles = cdt->GetTriangles();
    map = cdt->GetMap();

    for (std::vector<p2t::Triangle*>::iterator i = triangles.begin(); i != triangles.end(); ++i) {
        QVector4D c = getColor();

        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(0)->x, (*i)->GetPoint(0)->y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(1)->x, (*i)->GetPoint(1)->y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(2)->x, (*i)->GetPoint(2)->y, 0 ), c ) );

        unsigned long long x_1 = (unsigned long long)(*i)->GetPoint(0);
        x_1 = (unsigned long long)(*i)->GetPoint(1);
        x_1 = (unsigned long long)(*i)->GetPoint(2);
        x_1 = (unsigned long long)(*i)->GetPoint(2);

    }*/

    typedef std::vector<GLushort> contour_idxs_t;
    typedef std::vector<contour_idxs_t> contours_idxs_t;

    contour_idxs_t contour_idxs;

    contour_idxs.push_back(21);
    contour_idxs.push_back(22);
    contour_idxs.push_back(52);
    contour_idxs.push_back(14);
    contour_idxs.push_back(13);


    std::vector<p2t::Point*> polyline;
    std::vector<p2t::Triangle*> triangles;

    /*for ( contour_idxs_t::const_iterator i = contour_idxs.begin(); i != contour_idxs.end(); ++i ) {
        const QVector3D& vertex =  _fg._vertex_data[*i].vertex;
        polyline.push_back(new p2t::Point(vertex.x(), vertex.y()));
    }*/

    /*polyline.push_back(new p2t::Point(2.5556793212890625, 1.5971133708953857));
    polyline.push_back(new p2t::Point(2.560000419616699, 1.6000010967254639));
    polyline.push_back(new p2t::Point(2.559999942779541, 1.6000005006790161));
    polyline.push_back(new p2t::Point(2.559999704360962, 1.600000023841858));
    polyline.push_back(new p2t::Point(2.554903507232666, 1.5989861488342285));*/

    polyline.push_back(new p2t::Point(2.5556793212890625, 1.5971133708953857));
    polyline.push_back(new p2t::Point(2.5556793212890725, 1.6971133708953857));
    polyline.push_back(new p2t::Point(2.5556793212890625, 1.7971133708953857));

    /*polyline.push_back(new p2t::Point(2.5556793212890625, 1.5971133708953857));
    polyline.push_back(new p2t::Point(2.6556793212890725, 1.6971133708953857));
    polyline.push_back(new p2t::Point(2.5556793212890625, 1.7971133708953857));*/

    for ( std::vector<p2t::Point*>::iterator i = polyline.begin(); i != polyline.end(); ++i ) {
        if (i != polyline.begin()) {
            p2t::Point* p = *i;
            p2t::Point* pp = *(i - 1);

            QVector3D v = QVector3D(p->x, p->y, 0);
            QVector3D vp = QVector3D(pp->x, pp->y, 0);

            if ( isEqualPoints( &v, &vp ) ) {
                int x = 0;
            }
        }
    }



    p2t::CDT* cdt = new p2t::CDT(polyline);

    cdt->Triangulate();
    triangles = cdt->GetTriangles();

    for (std::vector<p2t::Triangle*>::iterator i = triangles.begin(); i != triangles.end(); ++i) {
        QVector4D c = getColor();

        /*if ( (*i)->GetPoint(0)->idx == -1 ||
             (*i)->GetPoint(1)->idx == -1 ||
             (*i)->GetPoint(2)->idx == -1 ) {
            continue;
        }*/

        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(0)->x, (*i)->GetPoint(0)->y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(1)->x, (*i)->GetPoint(1)->y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(2)->x, (*i)->GetPoint(2)->y, 0 ), c ) );
    }

    for ( std::vector<p2t::Point*>::iterator i = polyline.begin(); i != polyline.end(); ++i ) {
        delete *i;
    }

    delete cdt;

}

void FluidGeometryTests::testDelaunay() {
    std::vector<Vector2<float>> points;

    points.push_back( Vector2<float>(2.3, 1.5) );
    points.push_back( Vector2<float>(2.7, 1.5) );
    points.push_back( Vector2<float>(2.7, 1.9) );
    points.push_back( Vector2<float>(2.6, 1.7) );
    //points.push_back( Vector2<float>(2.65, 1.55) );
    points.push_back( Vector2<float>(2.3, 1.9) );

    Delaunay<float> triangulation;
    std::vector<Triangle<float>> triangles = triangulation.triangulate(points);

    for (std::vector<Triangle<float>>::iterator i = triangles.begin(); i != triangles.end(); ++i) {
        QVector4D c = getColor();

        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( i->p1.x, i->p1.y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( i->p2.x, i->p2.y, 0 ), c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( i->p3.x, i->p3.y, 0 ), c ) );

    }
}

QVector4D FluidGeometryTests::getColor() {
    QVector4D color;

    _count_color++;

    if ( _count_color == 1 ) {
        color = QVector4D(1.0, 0.0, 0.0, 1.0);
    } else if ( _count_color == 2 ) {
        color = QVector4D(0.0, 1.0, 0.0, 1.0);
    } else if ( _count_color == 3 ) {
        color = QVector4D(0.0, 0.0, 1.0, 1.0);
    } else if ( _count_color == 4 ) {
        color = QVector4D(0.0, 0.0, 0.0, 1.0);
    } else if ( _count_color == 5 ) {
        color = QVector4D(0.0, 1.0, 1.0, 1.0);
        _count_color = 0;
    }
    return color;
}
