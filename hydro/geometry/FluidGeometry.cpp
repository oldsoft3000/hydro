#include "FluidGeometry.h"
#include "FluidTriangles.h"
#include "FluidGeometryUtils.h"
#include "unit_tests/FluidGeometryTests.h"
#include "GlobalParams.h"
#include "Interpolator.h"
#include "Game.h"
#include <limits>
#include <deque>
#include <functional>
#ifdef MEASURES
#include "Ticks.h"
#endif

#include <WorldController.h>
#include <chrono>

#include "Triangulate.h"

const float width_cell = 0.05;
const int area_num_x = 500;
const int area_num_y = 500;
const unsigned int outline_stride = 5;
const float normal_length = 0.04;
//const unsigned int outline_stride = 2;
//const float normal_length = 0.02;
const int num_active_rows = 3;


FluidGeometry::FluidGeometry() : _area(area_num_x, area_num_y, width_cell)
{
    _size_line = 0;
    _frame_counter = 0;
    _idx_max_row = 0;

}

void FluidGeometry::init(unsigned int outline_points_size) {
    if (outline_points_size % outline_stride) {
       _size_line = outline_points_size / outline_stride + 1;
    } else {
       _size_line = outline_points_size / outline_stride;
    }

    _outline_normals.clear();
    for ( int i = 0; i != _size_line; ++i ) {
        _outline_normals.push_back( OutlineNormal(_outline_normals) );
    }
    _buf_idxs_rad.resize(_size_line);

    _area.initialize();
}

void FluidGeometry::drawFluid(QOpenGLShaderProgram *program,
                              const std::vector<QVector2D>& outline_points) {

    generateGeometry(outline_points);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glLineWidth(1);
    drawTriangles(program, _vertex_data, _index_data);

    drawTriangles(program, _vertex_data_triangles);

    glLineWidth(2);
    drawPolyLine(program, _vertex_data_lines);
}


std::vector<OutlineNormal>::iterator FluidGeometry::getRadialIntersection(std::vector<OutlineNormal>::iterator inormal,
                                                                          const sectors_t& sectors,
                                                                          QVector3D& nearest_intersection,
                                                                          float& min_length_sqr) {
    std::vector<unsigned int>& cell_idxs = _buf_idxs_rad[inormal->idx];
    QVector3D intersection;
    std::vector<OutlineNormal>::iterator inearest_normal = _outline_normals.end();
    min_length_sqr = std::numeric_limits<float>::max();

    auto isNormalsInOneSector = [this,&sectors] (std::vector<OutlineNormal>::iterator inormal_0,
                                                 std::vector<OutlineNormal>::iterator inormal_1) -> bool {
        sectors_t::const_iterator isector = sectors.begin();

        for (; isector != sectors.end(); ++isector) {
            if (FluidGeometryUtils::isSectorsSplitted(_outline_normals,
                                                      inormal_0,
                                                      inormal_1,
                                                      isector->first,
                                                      isector->second)) {
                return false;
            }
        }
        return true;
    };

    for (std::vector<unsigned int>::iterator icell = cell_idxs.begin(); icell != cell_idxs.end(); ++icell) {
        SnippetArea::Cell* cell = _area.getCell(*icell);
        Q_ASSERT(cell);
        if (cell) {
            for (std::vector<unsigned int>::iterator idx_normal = cell->data_idxs.begin(); idx_normal != cell->data_idxs.end(); ++idx_normal) {
                if (*idx_normal != (unsigned)inormal->idx) {

                    OutlineNormals::iterator inormal_founded = getOutlineNormal(*idx_normal);
                    Q_ASSERT(inormal_founded != _outline_normals.end());

                    if ( std::find_if( inormal_founded->links_rad.begin(),
                                       inormal_founded->links_rad.end(),
                                       [&inormal] (const OutlineNormalLink& link) {
                                            return link.idx_normal == inormal->idx; }) != inormal_founded->links_rad.end() ) {
                        continue;
                    }

                    if (inormal_founded != _outline_normals.end()) {
                        bool is_intersected = false;

                        is_intersected = getSnippetsInstersection(inormal->vertex_begin,
                                                                  inormal->vertex_end,
                                                                  inormal_founded->vertex_begin,
                                                                  inormal_founded->vertex_end,
                                                                  intersection);

                        if (is_intersected) {
                            if (!isNormalsInOneSector(inormal, inormal_founded)) {
                                continue;
                            }

                            float length_sqr = (intersection - inormal->vertex_begin).lengthSquared();
                            if (length_sqr < min_length_sqr || (length_sqr == min_length_sqr && !inormal_founded->links_rad.empty())) {
                                min_length_sqr = length_sqr;
                                nearest_intersection = intersection;
                                inearest_normal = inormal_founded;
                                //return inearest_normal;
                            }
                        }
                    }
                }
            }
        }
    }

    return inearest_normal;

}

void FluidGeometry::linkNormalsRadial(OutlineNormals::iterator inormal_0,
                                      OutlineNormals::iterator inormal_1,
                                      QVector3D& intersection) {

    if ( !inormal_1->isClosed() ) {

        inormal_0->links_rad.push_back(OutlineNormalLink());
        inormal_0->links_rad.back().idx_normal = inormal_1->idx;
        inormal_1->links_rad.push_back(OutlineNormalLink());
        inormal_1->links_rad.back().idx_normal = inormal_0->idx;

        inormal_0->vertex_end = intersection;
        inormal_1->vertex_end = intersection;

        inormal_0->is_closed = true;
        inormal_1->is_closed = true;

        inormal_0->links_rad.back().type = inormal_1->links_rad.back().type = LINK_RAD_EQUAL;
    } else {
        auto doLink = [this, &intersection] (OutlineNormals::iterator inormal_0,
                                             OutlineNormals::iterator inormal_1,
                                             bool is_equal_ends) {

            inormal_0->links_rad.push_back(OutlineNormalLink());
            inormal_0->links_rad.back().idx_normal = inormal_1->idx;
            inormal_1->links_rad.push_back(OutlineNormalLink());
            inormal_1->links_rad.back().idx_normal = inormal_0->idx;

            if ( is_equal_ends ) {
                inormal_0->links_rad.back().type = inormal_1->links_rad.back().type = LINK_RAD_EQUAL;
            } else {
                inormal_0->links_rad.back().type = LINK_RAD_BLOCK;
                inormal_1->links_rad.back().type = LINK_RAD_BRANCH;
                inormal_0->vertex_end = intersection;

                inormal_1->links_rad.back().idx_row = ( inormal_1->vertex_begin - intersection ).length() / normal_length;
            }


            inormal_0->is_closed = true;
        };

        bool is_equal_ends = isEqualPoints( &intersection, &inormal_1->vertex_end );

        for ( std::vector<OutlineNormalLink>::iterator ilink = inormal_1->links_rad.begin(); ilink != inormal_1->links_rad.end(); ++ilink ) {
            if ( is_equal_ends ) {
                if ( ilink->type == LINK_RAD_EQUAL ) {
                    doLink(inormal_0, getOutlineNormal( ilink->idx_normal ), true);
                } else if ( ilink->type == LINK_RAD_BLOCK ) {
                    doLink(inormal_0, getOutlineNormal( ilink->idx_normal ), false);
                }
            } else {
                if ( ilink->type == LINK_RAD_BRANCH ) {
                    OutlineNormals::iterator ilinked_normal = getOutlineNormal( ilink->idx_normal );

                    if ( isEqualPoints( &intersection, &ilinked_normal->vertex_end ) ) {
                        doLink(inormal_0, ilinked_normal, true);
                    }
                }
            }

        }
        doLink(inormal_0, inormal_1, is_equal_ends);
    }
}

void FluidGeometry::linkNormalsTangen(OutlineNormals::iterator inormal,
                                      OutlineNormals::iterator inormal_r,
                                      OutlineNormals::iterator inormal_l,
                                      QVector3D& intersection) {

    inormal->link_tan.idx_normal_r = inormal_r->idx;
    inormal->link_tan.idx_normal_l = inormal_l->idx;
    inormal->vertex_end = intersection;

    inormal_r->links_tan_l.push_back(OutlineNormalLink());
    inormal_r->links_tan_l.back().idx_normal_pair = inormal_l->idx;
    inormal_r->links_tan_l.back().idx_normal = inormal->idx;
    inormal_r->links_tan_l.back().idx_row = _idx_max_row;
    inormal_r->links_tan_l.back().type = LINK_TAN;

    inormal_l->links_tan_r.push_back(OutlineNormalLink());
    inormal_l->links_tan_r.back().idx_normal_pair = inormal_r->idx;
    inormal_l->links_tan_r.back().idx_normal = inormal->idx;
    inormal_l->links_tan_r.back().idx_row = _idx_max_row;
    inormal_l->links_tan_r.back().type = LINK_TAN;

    inormal->is_closed = true;

}


void FluidGeometry::updateAreaTangen() {

    auto processPair = [this] (OutlineNormals::iterator inormal_0,
                               OutlineNormals::iterator inormal_1)  {

        QVector3D vertex_end_0;
        QVector3D vertex_end_1;

        vertex_end_0 = inormal_0->vertex_end;
        vertex_end_1 = inormal_1->vertex_end;

        _buf_idxs_tan.clear();


        if (_area.getSnippetIdxs(vertex_end_0, vertex_end_1, _buf_idxs_tan)) {

            for (std::vector<unsigned int>::iterator idx_cell = _buf_idxs_tan.begin(); idx_cell != _buf_idxs_tan.end(); ++idx_cell) {
                SnippetArea::Cell* cell = _area.getCell(*idx_cell);
                Q_ASSERT(cell);

                if (cell && cell->frame_counter == _frame_counter) {

                    for (std::vector<unsigned int>::iterator idx_normal = cell->data_idxs.begin(); idx_normal != cell->data_idxs.end(); ++idx_normal) {
                        if (*idx_normal != (unsigned)inormal_0->idx && *idx_normal != (unsigned)inormal_1->idx) {

                            OutlineNormals::iterator inormal = getOutlineNormal(*idx_normal);
                            Q_ASSERT(inormal != _outline_normals.end());

                            if (inormal != _outline_normals.end()) {

                                if ( std::find_if( inormal->links_rad.begin(),
                                                   inormal->links_rad.end(),
                                                   [&inormal_0, &inormal_1] (const OutlineNormalLink& link) {
                                                        return link.idx_normal == inormal_0->idx || link.idx_normal == inormal_1->idx; }) != inormal->links_rad.end() ) {
                                    continue;
                                }

                                if ( std::find_if( inormal_0->links_tan_l.begin(),
                                                   inormal_0->links_tan_l.end(),
                                                   [&inormal] (const OutlineNormalLink& link) {
                                                        return link.idx_normal == inormal->idx; }) != inormal_0->links_tan_l.end() ) {
                                    continue;
                                }

                                QVector3D tangen_intersection;

                                if (getSnippetsInstersection(vertex_end_0,
                                                             vertex_end_1,
                                                             inormal->vertex_begin,
                                                             inormal->vertex_end,
                                                             tangen_intersection)) {

                                    linkNormalsTangen(inormal,
                                                inormal_0,
                                                inormal_1,
                                                tangen_intersection);

                                }
                            }
                        }
                    }
                }
            }
        }
    };

    std::vector<OutlineNormal>::iterator inormal = _outline_normals.begin();
    std::vector<OutlineNormal>::iterator inormal_prev = _outline_normals.end();
    std::vector<OutlineNormal>::iterator inormal_base = _outline_normals.end();


    while (inormal != _outline_normals.end()) {

        if (inormal_prev == _outline_normals.end()) {
            inormal_base = inormal;
        } else {

            processPair(inormal_prev,
                        inormal);

        }

        inormal_prev = inormal;
        inormal++;
    }

    if (inormal_prev != _outline_normals.end() && inormal_base != _outline_normals.end()) {
        processPair(inormal_prev,
                    inormal_base);

    }

}

void FluidGeometry::updateAreaRadial() {

    sectors_t sectors;

    std::vector<OutlineNormal>::iterator inormal = _outline_normals.begin();

    while (inormal != _outline_normals.end()) {
        if (inormal->isClosed() == true) {
            inormal++;
            continue;
        }

        QVector3D nearest_intersection;
        float min_length;

        std::vector<OutlineNormal>::iterator imin_normal = getRadialIntersection(inormal, sectors, nearest_intersection, min_length);

        if (imin_normal != _outline_normals.end()) {
            linkNormalsRadial(inormal, imin_normal, nearest_intersection);

            sectors.push_back(std::make_pair(inormal, imin_normal));
        }

        inormal++;
    }

    //qDebug() << "max_count " << max_count;
}

void FluidGeometry::initOutlineNormals(const std::vector<QVector2D>& outline_points) {
    std::vector<OutlineNormal>::iterator inormal = _outline_normals.begin();

    const QVector3D vec_z(0, 0, -1);

    for (; inormal != _outline_normals.end(); ++inormal) {
        QVector3D vertex;
        QVector3D vertex_next;
        unsigned int idx_normal = inormal - _outline_normals.begin();

        vertex = outline_points[idx_normal * outline_stride];

        if (inormal != _outline_normals.end() - 1) {
            vertex_next = outline_points[(idx_normal + 1) * outline_stride];
        } else {
            vertex_next = outline_points[0];
        }

        QVector3D vec_point = vertex_next - vertex;

        QVector3D normal = QVector3D::normal(vec_point, vec_z);
        normal.normalize();
        normal = normal * normal_length;
        inormal->normal = normal;

        QVector3D vertex_0 = QVector3D((vertex.x() + vertex_next.x()) / 2, (vertex.y() + vertex_next.y()) / 2, 0);
        /*addVertex(vertex_0);
        _outline_indexes[idx_normal].push_back(_buf_vertex.size() - 1);*/
        inormal->vertex_begin = inormal->vertex_end = vertex_0;
        inormal->idx = idx_normal;
        inormal->is_closed = false;

        inormal->kZ = 0;
        inormal->kL = 0;


        inormal->links_rad.clear();
        inormal->link_tan = OutlineNormalLink();
        inormal->links_tan_r.clear();
        inormal->links_tan_l.clear();
        inormal->indicies.clear();

        int idx_outline_point = (inormal - _outline_normals.begin()) * outline_stride;
    }
    _idx_max_row = 0;
}

void FluidGeometry::clearBuffers() {

    _vertex_data.clear();
    _index_data.clear();
    _vertex_data_lines.clear();
    _vertex_data_triangles.clear();

    //_area.clear();
}



void FluidGeometry::generateGeometry(const std::vector<QVector2D>& outline_points) {


    clearBuffers();

    float area_left = std::numeric_limits<float>::max();
    float area_top = std::numeric_limits<float>::max();

    //int idx_point = 0;

    for (std::vector<QVector2D>::const_iterator ipoint = outline_points.begin(); ipoint != outline_points.end(); ++ipoint) {
        if ((ipoint - outline_points.begin()) % outline_stride == 0) {
            if (area_left > ipoint->x()) {
                area_left = ipoint->x();
            }
            if (area_top > ipoint->y()) {
                area_top = ipoint->y();
            }
        }
    }

    _area.setLeft(area_left);
    _area.setTop(area_top);

    initOutlineNormals(outline_points);


    while ( growNormals() ) {
        START_TIMER(MEASURE_UPDATE_RADIAL);
        updateAreaRadial();
        PAUSE_TIMER(MEASURE_UPDATE_RADIAL);

        START_TIMER(MEASURE_UPDATE_TANGEN);
        updateAreaTangen();
        PAUSE_TIMER(MEASURE_UPDATE_TANGEN);

        _idx_max_row++;
    }

    fillZ();
    fillVertices();

    START_TIMER(MEASURE_FILL_SECTORS);
    FluidTriangles triangles(*this);
    triangles.triangulate();
    PAUSE_TIMER(MEASURE_FILL_SECTORS);
    drawNormals();
    drawLine(_outline_normals[0].vertex_begin,
             _outline_normals[0].vertex_end,
             QColor(0, 255, 0, 255));

    //FluidGeometryTests tests(*this);
    //tests.testPoly2tri();*/

    _frame_counter++;

}


void FluidGeometry::growNormal(OutlineNormals::iterator inormal) {
    std::vector<unsigned int>& idxs_rad = _buf_idxs_rad[inormal->idx];

    int idx_row_1 = _idx_max_row + 1;
    int idx_row_0 = idx_row_1 - num_active_rows;

    if ( idx_row_0 < 0 ) {
        idx_row_0 = 0;
    }

    QVector3D vertex_0 = inormal->calcVertex( idx_row_0 );
    QVector3D vertex_1 = inormal->calcVertex( idx_row_1 );

    inormal->vertex_end = vertex_1;

    idxs_rad.clear();

    _area.addSnippet(_frame_counter,
                     inormal->idx,
                     idxs_rad,
                     vertex_0,
                     vertex_1);

}

unsigned int FluidGeometry::growNormals() {
    unsigned int c = 0;
    std::vector<OutlineNormal>::iterator inormal;

    for (inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal) {
        if (inormal->isClosed() == false) {
            growNormal(inormal);
            c++;
        }
    }
    return c;
}

void FluidGeometry::drawNormals() {
    std::vector<OutlineNormal>::iterator inormal;

    for (inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal) {

        if ( inormal->isClosed() ) {
            unsigned int num_segments = inormal->calcNumSegments();

            for (unsigned int idx_row = 0; idx_row < num_segments; ++idx_row) {
                QColor color;
                if (idx_row % 2 == 0) {
                    color = QColor(1.0, 0, 0, 1.0);
                } else {
                    color = QColor(1.0, 1.0, 1.0, 1.0);
                }

                if ( idx_row == num_segments - 1 ) {
                    drawLine(inormal->calcVertex( idx_row ),
                             inormal->vertex_end,
                             color);
                } else {
                    drawLine(inormal->calcVertex( idx_row ),
                             inormal->calcVertex( idx_row + 1 ),
                             color);
                }
            }
        } else {
            drawLine(inormal->calcVertex( 0 ),
                     inormal->calcVertex( 0 ),
                     QColor(0, 0, 0, 1.0));
        }
    }
}



void FluidGeometry::drawLine(const QVector3D& vertex_0, const QVector3D& vertex_1, const QColor& color) {
    _vertex_data_lines.push_back( vertex_t( vertex_0, QVector4D(color.red(), color.green(), color.blue(), color.alpha()) ) );
    _vertex_data_lines.push_back( vertex_t( vertex_1, QVector4D(color.red(), color.green(), color.blue(), color.alpha()) ) );
}


void FluidGeometry::addIndex(GLushort index, const QColor& color) {
    _index_data.push_back(index);
    _vertex_data[index].color = QVector4D((float)color.red() / 255.0, (float)color.green() / 255.0, (float)color.blue() / 255.0, (float)color.alpha() / 255.0);
}

void FluidGeometry::addVertex(const QVector3D& vertex) {
    _vertex_data.push_back(vertex);
}


void FluidGeometry::fillVertices() {
    std::vector<OutlineNormal>::iterator inormal;

    for (inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal) {
        if ( inormal->isClosed() ) {
            unsigned int num_segments = inormal->calcNumSegments();

             for ( unsigned int i = 0; i <= num_segments; i++ ) {
                int index = -1;
                if ( i == num_segments ) {
                    std::vector<OutlineNormalLink>::iterator ilink = inormal->links_rad.begin();

                    for ( ; ilink != inormal->links_rad.end(); ++ilink ) {
                        if ( ilink->type == LINK_RAD_EQUAL ) {
                            OutlineNormals::iterator ilinked_normal = getOutlineNormal( ilink->idx_normal );
                            if ( ilinked_normal != _outline_normals.end() && !ilinked_normal->indicies.empty() ) {
                                index = ilinked_normal->indicies.back();
                                inormal->indicies.push_back( index );
                                break;
                            }
                        }
                    }

                    if ( index != -1 ) {
                        continue;
                    }
                }
                _vertex_data.push_back( GeometryEngine::vertex_t(inormal->calcVertexZ( i )) );
                index = _vertex_data.size() - 1;
                inormal->indicies.push_back( index );

            }

        } else {
            Game::instance().getWorldController()->enableUpdates(false);
            //Q_ASSERT( false );
        }
    }
}

void FluidGeometry::fillZ() {
    std::vector<OutlineNormal>::iterator inormal;

    for ( inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal ) {
        inormal->kL = ( inormal->vertex_end - inormal->vertex_begin ).length();
    }

    for ( inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal ) {
        if ( inormal->kZ ) {
            continue;
        }

        float kZ = 0;

        if ( !inormal->links_rad.empty() ) {
            int count_links = 1;
            kZ = inormal->kL;
            for ( std::vector<OutlineNormalLink>::const_iterator ilink = inormal->links_rad.begin(); ilink != inormal->links_rad.end(); ++ilink ) {
                if ( ilink->type == LINK_RAD_EQUAL) {
                    OutlineNormals::const_iterator ilinked_normal = _outline_normals.begin() + ilink->idx_normal;
                    kZ += ilinked_normal->kL;
                    count_links++;
                }
            }
            inormal->kZ = kZ / count_links;

            for ( std::vector<OutlineNormalLink>::const_iterator ilink = inormal->links_rad.begin(); ilink != inormal->links_rad.end(); ++ilink, ++count_links ) {
                if ( ilink->type == LINK_RAD_EQUAL) {
                    OutlineNormals::iterator ilinked_normal = _outline_normals.begin() + ilink->idx_normal;
                    ilinked_normal->kZ = inormal->kZ;
                }
            }
        } else if ( inormal->link_tan.idx_normal_r != -1 && inormal->link_tan.idx_normal_l != -1 ) {
            OutlineNormals::const_iterator ilinked_normal_r = _outline_normals.begin() + inormal->link_tan.idx_normal_r;
            OutlineNormals::const_iterator ilinked_normal_l = _outline_normals.begin() + inormal->link_tan.idx_normal_l;
            inormal->kZ = ( inormal->kL + ilinked_normal_r->kL + ilinked_normal_l->kL ) / 3;
        }

    }
}

int FluidGeometry::getIdxVertex( int idx_normal, int idx_row ) const {
    if ( (unsigned)idx_normal >= _outline_normals.size() ) {
        return -1;
    }
    const OutlineNormal& normal = _outline_normals[idx_normal];

    if ( (unsigned)idx_row >= normal.indicies.size() ) {
        return normal.indicies.back();
    }

    return normal.indicies[idx_row];
}

