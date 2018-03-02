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
#include <memory>

#include <WorldController.h>
#include <chrono>

#include "Triangulate.h"

const float width_cell = 0.05;
const int area_num_x = 500;
const int area_num_y = 500;
const unsigned int outline_stride = 5;
const float normal_length = 1.0;
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
    for ( unsigned int i = 0; i != _size_line; ++i ) {
        _outline_normals.push_back( OutlineNormal(_outline_normals) );
    }
    _idxs_rad.resize(_size_line);

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
    std::vector<unsigned int>& cell_idxs = _idxs_rad[inormal->idx];
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

void FluidGeometry::updateAreaTangen() {

    struct intersection_t {
        QVector3D intersection;
        intersection_t() :  intersection(-1, -1, -1) {}
        intersection_t(const QVector3D& vector) :  intersection(vector.x(), vector.y(), vector.z()) {}
        bool is_set() { return intersection.x() != -1; }
    };
    std::vector<intersection_t> intersections;

    intersections.resize( _outline_normals.size() );

    auto processPair = [this, &intersections] (OutlineNormals::iterator inormal_0,
                                               OutlineNormals::iterator inormal_1)  {

        QVector3D vertex_end_0;
        QVector3D vertex_end_1;

        vertex_end_0 = inormal_0->vertex_end;
        vertex_end_1 = inormal_1->vertex_end;

        _idxs_tan.clear();

        if (_area.getSnippetIdxs(vertex_end_0, vertex_end_1, _idxs_tan)) {

            for (std::vector<unsigned int>::iterator idx_cell = _idxs_tan.begin(); idx_cell != _idxs_tan.end(); ++idx_cell) {
                SnippetArea::Cell* cell = _area.getCell(*idx_cell);
                Q_ASSERT(cell);

                if (cell && cell->frame_counter == _frame_counter) {
                    ///TODO sort tang links
                    for (std::vector<unsigned int>::iterator idx_normal = cell->data_idxs.begin(); idx_normal != cell->data_idxs.end(); ++idx_normal) {
                        if (*idx_normal != (unsigned)inormal_0->idx && *idx_normal != (unsigned)inormal_1->idx) {

                            OutlineNormals::iterator inormal = getOutlineNormal(*idx_normal);
                            Q_ASSERT(inormal != _outline_normals.end());

                            if (inormal != _outline_normals.end()) {

                                /*if ( std::find_if(  inormal->links_rad.begin(),
                                                          inormal->links_rad.end(),
                                                          [&inormal_0, &inormal_1] ( const OutlineNormalLink& link ) {
                                                                if ( link.idx_normal == inormal_0->idx && link.type == LINK_RAD_EQUAL ) {
                                                                        return true;
                                                                }
                                                                if ( link.idx_normal == inormal_1->idx && link.type == LINK_RAD_EQUAL ) {
                                                                        return true;
                                                                }
                                                                return false;
                                                        } ) != inormal->links_rad.end() ) {
                                    continue;
                                }*/

                                /*if ( std::find_if( inormal->links_rad.begin(),
                                                         inormal->links_rad.end(),
                                                          [] (const OutlineNormalLink& link)  {
                                                                return link.type == LINK_RAD_EQUAL; } ) != inormal->links_rad.end() ) {
                                    continue;
                                }*/

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

                                    intersections[inormal->idx] = tangen_intersection;

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

    int n = 0;
    for ( auto i : intersections) {
        if (i.is_set()) {
            _outline_normals[n].vertex_end = i.intersection;
        }
        n++;
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

void FluidGeometry::searchRadialInns() {
    OutlineNormals::iterator inormal_0, inormal_1;

    inormal_0 = _outline_normals.begin();

    while ( inormal_0 != _outline_normals.end() ) {
        inormal_1 = inormal_0 + 1;
        while ( inormal_1 != _outline_normals.end() ) {

            QVector3D point;
            InnWeightsT::iterator iinn;

            iinn = std::find_if( inormal_0->inns.begin(),
                                 inormal_0->inns.end(),
                                 [&inormal_1] ( const std::pair<InnPtr, float>& inn_weight ) -> bool {
                                       NrmWeightsT::iterator inrm_weight = inn_weight.first->getInnWeightByNormal( inormal_1->idx );
                                       if ( inrm_weight != inn_weight.first->normals.end() ) {
                                           return true;
                                       } else {
                                           return false;
                                       }
                                 } );

            if ( iinn == inormal_0->inns.end() ) {
                if (getSnippetsInstersection(inormal_0->vertex_begin,
                                             inormal_0->vertex_end,
                                             inormal_1->vertex_begin,
                                             inormal_1->vertex_end,
                                             point )) {

                    iinn = std::find_if( inormal_0->inns.begin(),
                                         inormal_0->inns.end(),
                                         [&point] ( const std::pair<InnPtr, float>& inn_weight ) -> bool {
                                             return isEqualPoints( &point, &inn_weight.first->point );
                                         } );

                    InnPtr inn;
                    float weight_0, weight_1;

                    if ( iinn == inormal_0->inns.end() ) {
                         inn = std::make_shared<InnT>();
                         inn->point = point;
                         weight_0 = ( point - inormal_0->vertex_begin ).lengthSquared();
                         weight_1 = ( point - inormal_1->vertex_begin ).lengthSquared();
                         inormal_0->inns.push_back( std::make_pair( inn, weight_0 ) );
                         inormal_1->inns.push_back( std::make_pair( inn, weight_1 ) );
                         inn->normals.push_back( std::make_pair( inormal_0->idx, weight_0 ) );
                         inn->normals.push_back( std::make_pair( inormal_1->idx, weight_1 ) );
                         _inns.push_back( inn );
                    } else {
                         inn = iinn->first;
                         weight_0 = inn->max_weight;
                         weight_1 = ( point - inormal_1->vertex_begin ).lengthSquared();
                         inn->normals.push_back( std::make_pair( inormal_1->idx, weight_1 ) );
                    }
                    if ( weight_0 < weight_1 ) std::swap(  weight_0, weight_1 );
                    inn->max_weight = weight_0;
                    inn->max_koef = weight_0 / weight_1;
                }
            }
            inormal_1++;
        }
        inormal_0++;
    }

    std::sort( _inns.begin(),
               _inns.end(),
                [](const InnPtr & a, const InnPtr & b) -> bool
                {
                    return a->max_weight < b->max_weight;
                } );
}

void FluidGeometry::shrinkRadialInns() {
    for ( auto inn : _inns ) {
        if ( inn->state == InnState::UNKNOWN ) {
            for ( auto nrm_weight : inn->normals ) {
                OutlineNormals::iterator inormal = getOutlineNormal( nrm_weight.first );
                if ( inormal->is_closed == false ) {
                    NrmWeightsT::iterator inrm_weight = inn->getInnWeightByNormal( inormal->idx );
                    Q_ASSERT( inrm_weight != inn->normals.end() );
                    InnWeightsT::iterator iinn = inormal->inns.begin();
                    while ( iinn != inormal->inns.end() ) {
                        if ( iinn->first != inn ) {
                            if ( iinn->second > inrm_weight->second ) {
                                iinn->first->state = InnState::DELETED;
                            }
                        }
                        iinn++;
                    }
                    inormal->vertex_end = inn->point;
                    inormal->is_closed = true;
                }
            }
            inn->state = InnState::FIXED;

            if ( inn->max_koef > 1.5 && inn->normals.size() == 2 ) {
                OutlineNormals::iterator inormal_0 = getOutlineNormal( inn->normals[0].first );
                OutlineNormals::iterator inormal_1 = getOutlineNormal( inn->normals[1].first );

                float length_0 = ( inn->point - inormal_0->vertex_begin ).length();
                float length_1 = ( inn->point - inormal_1->vertex_begin ).length();

                float x = ( length_0 - length_1 ) / 2 ;

                if ( isEqualPoints( &inn->point, &inormal_0->vertex_end ) ) {
                    inormal_0->vertex_end = inormal_0->vertex_begin + inormal_0->normal * ( length_0 - x );
                    inormal_0->is_closed = false;
                    inn->state = InnState::EDITED;
                }
                if ( isEqualPoints( &inn->point, &inormal_1->vertex_end ) ) {
                    inormal_1->vertex_end = inormal_1->vertex_begin + inormal_1->normal * ( length_1 + x );
                    inormal_1->is_closed = false;
                    inn->state = InnState::EDITED;
                }
            }
        }
    }

    OutlineNormals::iterator  inormal = _outline_normals.begin();

    while ( inormal != _outline_normals.end() ) {
        if ( inormal->is_closed == false ) {
            inormal->inns.clear();
        } else {
            InnWeightsT::iterator iinn = inormal->inns.begin();
            while ( iinn != inormal->inns.end() ) {
                if ( iinn->first->state != InnState::FIXED ) {
                    iinn = inormal->inns.erase( iinn );
                } else {
                    iinn++;
                }
            }
        }
        inormal++;
    }
}

void FluidGeometry::searchTangenInns() {
    auto isPeak = [this] (OutlineNormals::iterator inormal) -> bool {
        InnWeightsT::iterator iinn = inormal->inns.begin();
        while ( iinn != inormal->inns.end() ) {
            if ( iinn->first->normals.size() == 2 ) {
                OutlineNormals::iterator inormal_0 = getOutlineNormal( iinn->first->normals[0].first );
                OutlineNormals::iterator inormal_1 = getOutlineNormal( iinn->first->normals[1].first );

                if ( isEqualPoints( &iinn->first->point, &inormal_0->vertex_end ) &&
                     isEqualPoints( &iinn->first->point, &inormal_1->vertex_end ) &&
                     std::abs(distance_cycled( _outline_normals.begin(),
                                               _outline_normals.end(),
                                               inormal_0, inormal_1 )) == 1 ) {

                    return true;
                }
            }
            iinn++;
        }
        return false;
    };

    auto cutOuterContour = [this] (const QVector3D& v_0,
                                   const QVector3D& v_1)  {
        OutlineNormals::iterator inormal = _outline_normals.begin();

        while ( inormal != _outline_normals.end() ) {
            QVector3D point;
            if (getSnippetsInstersection(v_0,
                                         v_1,
                                         inormal->vertex_begin,
                                         inormal->vertex_end,
                                         point )) {
                float length = ( point - inormal->vertex_begin ).length();
                inormal->vertex_end = inormal->vertex_begin + inormal->normal * length / 2;
                inormal->is_closed = true;
            }
            inormal++;
        }
    };

    auto cutInnerContour = [this, canCut] (const QVector3D& v_0,
                                           const QVector3D& v_1)  {
        OutlineNormals::iterator inormal = _outline_normals.begin();
        std::vector<std::pair<OutlineNormals::iterator, QVector3D>> founded;
        while ( inormal != _outline_normals.end() ) {
            if ( canCut( inormal ) ) {
                QVector3D point;
                if (getSnippetsInstersection(v_0,
                                             v_1,
                                             inormal->vertex_begin,
                                             inormal->vertex_end,
                                             point )) {
                    founded.push_back( std::make_pair( inormal, point ) );
                }
            }
            inormal++;
        }

        for ( auto normal_point : founded ) {
            normal_point.first->vertex_end = normal_point.second;
            normal_point.first->is_closed = true;
        }
    };


    std::vector<OutlineNormal>::iterator inormal = _outline_normals.begin();
    std::vector<OutlineNormal>::iterator inormal_prev = _outline_normals.end();
    std::vector<OutlineNormal>::iterator inormal_base = _outline_normals.end();

    while (inormal != _outline_normals.end()) {
        /*if ( canCut( inormal ) ) {
            inormal++;
            continue;
        }*/

        if (inormal_prev == _outline_normals.end()) {
            inormal_base = inormal;
        } else {
            cutInnerContour( inormal_prev->vertex_end, inormal->vertex_end );
        }

        inormal_prev = inormal;
        inormal++;
    }

    if (inormal_prev != _outline_normals.end() && inormal_base != _outline_normals.end()) {
        cutInnerContour( inormal_prev->vertex_end, inormal_base->vertex_end );
    }

    inormal = _outline_normals.begin();
    inormal_prev = _outline_normals.end();
    inormal_base = _outline_normals.end();

    unsigned int count_closed = std::count_if( _outline_normals.begin(),
                                      _outline_normals.end(),
                                      []( const OutlineNormal& normal )->bool {
                                            return normal.is_closed;
                                      });

    if ( count_closed != _outline_normals.size() ) {
        inormal = _outline_normals.begin();
        inormal_prev = _outline_normals.end();
        inormal_base = _outline_normals.end();

        while (inormal != _outline_normals.end()) {
            if (inormal_prev == _outline_normals.end()) {
                inormal_base = inormal;
            } else {
                cutOuterContour( inormal_prev->vertex_begin, inormal->vertex_begin );
            }

            inormal_prev = inormal;
            inormal++;
        }

        if (inormal_prev != _outline_normals.end() && inormal_base != _outline_normals.end()) {
            cutOuterContour( inormal_prev->vertex_begin, inormal_base->vertex_begin );
        }
    }
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

        for ( OutlineNormalLinks::iterator ilink = inormal_1->links_rad.begin(); ilink != inormal_1->links_rad.end(); ++ilink ) {
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

    /*if ( std::find_if( inormal->links_rad.begin(),
                             inormal->links_rad.end(),
                              [] (const OutlineNormalLink& link)  {
                                    return link.type == LINK_RAD_EQUAL; } ) == inormal->links_rad.end() ) {
         inormal->vertex_end = intersection;
    }*/
    //inormal->vertex_end = intersection;

    int idx_link_row = std::max( inormal_l->calcNumSegments() - 1, inormal_r->calcNumSegments() - 1);

    //idx_link_row = _idx_max_row;

    inormal_r->links_tan_l.push_back(OutlineNormalLink());
    inormal_r->links_tan_l.back().idx_normal_pair = inormal_l->idx;
    inormal_r->links_tan_l.back().idx_normal = inormal->idx;
    inormal_r->links_tan_l.back().idx_row = idx_link_row;
    inormal_r->links_tan_l.back().type = LINK_TAN;

    inormal_l->links_tan_r.push_back(OutlineNormalLink());
    inormal_l->links_tan_r.back().idx_normal_pair = inormal_r->idx;
    inormal_l->links_tan_r.back().idx_normal = inormal->idx;
    inormal_l->links_tan_r.back().idx_row = idx_link_row;
    inormal_l->links_tan_r.back().type = LINK_TAN;

    inormal->is_closed = true;

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
        //normal = normal * normal_length;
        inormal->normal = normal;

        QVector3D vertex_0 = QVector3D((vertex.x() + vertex_next.x()) / 2, (vertex.y() + vertex_next.y()) / 2, 0);

        inormal->vertex_begin = vertex_0;
        inormal->vertex_end = inormal->vertex_begin + normal;
        inormal->idx = idx_normal;
        inormal->is_closed = false;

        inormal->kZ = 0;
        inormal->kL = 0;

        inormal->links_rad.clear();
        inormal->link_tan = OutlineNormalLink();
        inormal->links_tan_r.clear();
        inormal->links_tan_l.clear();
        inormal->indicies.clear();
        inormal->inns.clear();
    }
    _idx_max_row = 0;
}

void FluidGeometry::clearBuffers() {

    _vertex_data.clear();
    _index_data.clear();
    _vertex_data_lines.clear();
    _vertex_data_triangles.clear();
    _inns.clear();

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

    searchRadialInns();
    shrinkRadialInns();
    searchTangenInns();

    /*while ( growNormals()  && _idx_max_row < 20 ) {
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
    sortLinks()

    START_TIMER(MEASURE_FILL_SECTORS);
    FluidTriangles triangles(*this);
    triangles.triangulate();
    PAUSE_TIMER(MEASURE_FILL_SECTORS);;*/

    drawNormals();
    drawLine(_outline_normals[0].vertex_begin,
             _outline_normals[0].vertex_end,
             QColor(0, 255, 0, 255));

    //FluidGeometryTests tests(*this);
    //tests.testPoly2tri();*/

    _frame_counter++;

}

void FluidGeometry::sortLinks() {
    std::vector<OutlineNormal>::iterator inormal;

    for ( inormal = _outline_normals.begin(); inormal != _outline_normals.end(); ++inormal ) {
        if ( inormal->links_tan_r.size() > 1 ) {
            inormal->links_tan_r.sort(
                [&inormal, this](const OutlineNormalLink & a, const OutlineNormalLink & b) -> bool
                {
                    int d_1, d_2;
                    if ( a.idx_normal > inormal->idx ) {
                        d_1 = a.idx_normal - inormal->idx;
                    } else {
                        d_1 = a.idx_normal - inormal->idx + _outline_normals.size();
                    }

                    if ( b.idx_normal > inormal->idx ) {
                        d_2 = b.idx_normal - inormal->idx;
                    } else {
                        d_2 = b.idx_normal - inormal->idx + _outline_normals.size();
                    }

                    return d_1 > d_2;
                });
        }
    }
}

void FluidGeometry::growNormal(OutlineNormals::iterator inormal) {
    std::vector<unsigned int>& idxs_rad = _idxs_rad[inormal->idx];

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

        //if ( inormal->isClosed() ) {
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
        /*} else {
            drawLine(inormal->calcVertex( 0 ),
                     inormal->calcVertex( 0 ),
                     QColor(0, 0, 0, 1.0));
        }*/
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
                    OutlineNormalLinks::iterator ilink = inormal->links_rad.begin();

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
            for ( OutlineNormalLinks::iterator ilink = inormal->links_rad.begin(); ilink != inormal->links_rad.end(); ++ilink ) {
                if ( ilink->type == LINK_RAD_EQUAL) {
                    OutlineNormals::const_iterator ilinked_normal = _outline_normals.begin() + ilink->idx_normal;
                    kZ += ilinked_normal->kL;
                    count_links++;
                }
            }
            inormal->kZ = kZ / count_links;

            for ( OutlineNormalLinks::iterator ilink = inormal->links_rad.begin(); ilink != inormal->links_rad.end(); ++ilink, ++count_links ) {
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

