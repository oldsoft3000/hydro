#include "FluidTriangles.h"
#include "FluidGeometryUtils.h"
#include "Triangulate.h"
#include "CommonUtils.h"
#include "Game.h"
#include "ColorGenerator.h"
#include <WorldController.h>
#include <poly2tri.h>
#include <functional>
#include <utility>
#include <algorithm>

FluidTriangles::FluidTriangles(FluidGeometry& fluid_geometry) : _fg(fluid_geometry), _edges(fluid_geometry)
{
    _color_generator = std::make_shared<ColorGenerator>();
}

void FluidTriangles::triangulate() {

    /*for (std::vector<path_data_t>::iterator idata = _path_data.begin(); idata != _path_data.end(); ++idata ) {
        idata->edges.clear();
    }*/

    unsigned int idx_max_row = _fg._idx_max_row;
    contours_idxs_t contours_idxs;

    for (unsigned int idx_row = 0; idx_row < idx_max_row; idx_row++) {
        OutlineNormals::iterator inormal, inormal_next;
        OutlineNormals::iterator inormal_0 = inormal = findFirstNormal( 0, idx_row );

        if ( inormal != _fg._outline_normals.end() ) {
            contour_idxs_t contour_idxs;
            SCR scr;

            do {
                scr = findContour( idx_row, inormal, contour_idxs, inormal_next );
                if ( scr == SCR::SUCCESS || scr == SCR::REPEAT ) {
                    inormal = inormal_next;
                    processContour( contour_idxs, QColor(128, 128, 128, 255) );
                } else if ( scr == SCR::SEARCH ) {
                    inormal = findFirstNormal( inormal->idx + 1, idx_row );
                } else if ( scr == SCR::ERROR ) {
                    return;
                }
            } while ( ( inormal != inormal_0 || scr == SCR::REPEAT ) && inormal != _fg._outline_normals.end() );
        }
    }

    /*for ( GeometryEngine::vertex_data_t::iterator idata = _fg._vertex_data.begin(); idata != _fg._vertex_data.end(); ++idata ) {
        idata->normal.normalize();

        QVector3D normal = idata->normal / 30;
        _fg.drawLine( idata->vertex, idata->vertex + normal, QColor(0, 0, 255, 255) );
    }*/

}


void FluidTriangles::processSector(int idx_row, const sector_t& sector) {
    contour_idxs_t contour_idxs;

    sector_t spliter(_fg._outline_normals.end(), _fg._outline_normals.end());

    /// prev row
    OutlineNormals::iterator inormal = inc_cycled(_fg._outline_normals.begin(), _fg._outline_normals.end(), sector.inormal_r);
    while (inormal != sector.inormal_l) {
        if ( getOffsetFromEnd( idx_row, inormal ) == 0 ) {
            addToContour( -1, inormal->idx, contour_idxs );
        }
        inormal = inc_cycled(_fg._outline_normals.begin(), _fg._outline_normals.end(), inormal);
    }

    /// left normal
    if (sector.inormal_l != _fg._outline_normals.end()) {
        addToContour(idx_row, sector.inormal_l->idx, contour_idxs);
        addToContour(idx_row + 1, sector.inormal_l->idx, contour_idxs);
    }

    /// next row
    if ( getOffsetFromEnd(idx_row, sector.inormal_r) <= 1 ) {
        for (OutlineNormalLinks::const_reverse_iterator ilink = sector.inormal_r->links_tan_l.rbegin(); ilink != sector.inormal_r->links_tan_l.rend(); ++ilink) {
            addToContour( -1, ilink->idx_normal, contour_idxs );
        }
    }

    /// right normal
    if (sector.inormal_r != _fg._outline_normals.end()) {
        addToContour(idx_row + 1, sector.inormal_r->idx, contour_idxs);
        addToContour(idx_row, sector.inormal_r->idx, contour_idxs);
    }

    processContour(contour_idxs, QColor(128, 128, 128, 255));


}

void FluidTriangles::processContour(contour_idxs_t& contour_idxs, const QColor& color) {
    if ( contour_idxs.size() >= 3 ) {
        contour_idxs_t::const_iterator iidx_prev = contour_idxs.end() - 2;
        contour_idxs_t::const_iterator iidx_curr = contour_idxs.end() - 1;

        QVector3D vertex_prev;
        QVector3D vertex_curr;
        QVector3D vertex_next;
        QVector3D cross;

        for ( contour_idxs_t::const_iterator iidx_next = contour_idxs.begin(); iidx_next != contour_idxs.end(); iidx_next++ ) {
            vertex_prev = _fg._vertex_data[*iidx_prev].vertex;
            vertex_curr = _fg._vertex_data[*iidx_curr].vertex;
            vertex_next = _fg._vertex_data[*iidx_next].vertex;

            cross = QVector3D::crossProduct( vertex_next - vertex_curr, vertex_prev - vertex_curr );
            cross.normalize();
            _fg._vertex_data[*iidx_curr].normal += cross;
            _fg._vertex_data[*iidx_curr].normal.normalize();

            //edges_t& edges_curr = _path_data[*iidx_curr].edges;

            /*if (std::find( edges_curr.begin(), edges_curr.end(), *iidx_prev) == edges_curr.end() ) {
                edges_curr.push_back( *iidx_prev );
            }
            if (std::find( edges_curr.begin(), edges_curr.end(), *iidx_next) == edges_curr.end() ) {
                edges_curr.push_back( *iidx_next );
            }*/

            _fg.drawLine( vertex_prev, vertex_curr, QColor(0, 0, 0, 255) );
            _fg.drawLine( vertex_curr, vertex_next, QColor(0, 0, 0, 255) );

            iidx_prev = iidx_curr;
            iidx_curr = iidx_next;


        }
    }

    const GeometryEngine::vertex_data_t& vertex_data = _fg._vertex_data;
    std::vector<unsigned short> result;

    Triangulate::Process((QVector3D*)_fg._vertex_data.data(), sizeof(GeometryEngine::vertex_t), contour_idxs, result);

    /*for (Vector2dVector::const_iterator ivector = contour.begin();
         ivector != contour.end();
         ivector++) {

        _fg.addVertex(QVector3D(ivector->GetX(), ivector->GetY(), 0));
    }*/

    unsigned int tcount = result.size() / 3;

    for (unsigned int i = 0; i < tcount; i++) {
        QVector4D c = _color_generator->getColor();

        _fg.addIndex(result[i * 3 + 0], color);
        _fg.addIndex(result[i * 3 + 1], color);
        _fg.addIndex(result[i * 3 + 2], color);

        /*_fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( _fg._vertex_data[result[i * 3 + 0]].vertex, c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( _fg._vertex_data[result[i * 3 + 1]].vertex, c ) );
        _fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( _fg._vertex_data[result[i * 3 + 2]].vertex, c ) );*/
    }

    /*{
        contour_idxs_t::iterator i, j ;
        std::set< int > t_set;
        for( i = contour_idxs.begin(), j = contour_idxs.begin() ; i != contour_idxs.end() ; ++i )
            if( t_set.insert( *i ).second)
                *j++ = *i ;
        contour_idxs.erase( j , contour_idxs.end() );
    }*/

    std::vector<p2t::Point*> polyline;
    std::vector<p2t::Triangle*> triangles;

    /*for ( contour_idxs_t::const_iterator i = contour_idxs.begin(); i != contour_idxs.end(); ++i ) {
        const QVector3D& vertex =  _fg._vertex_data[*i].vertex;
        polyline.push_back(new p2t::Point(vertex.x(), vertex.y()));
    }

    p2t::CDT* cdt = new p2t::CDT(polyline);

    cdt->Triangulate();
    triangles = cdt->GetTriangles();*/

    for (std::vector<p2t::Triangle*>::iterator i = triangles.begin(); i != triangles.end(); ++i) {
        QVector4D c = _color_generator->getColor();

        /*if ( (*i)->GetPoint(0)->idx == -1 ||
             (*i)->GetPoint(1)->idx == -1 ||
             (*i)->GetPoint(2)->idx == -1 ) {
            continue;
        }*/

        /*_fg.addIndex((*i)->GetPoint(0)->idx, color);
        _fg.addIndex((*i)->GetPoint(1)->idx, color);
        _fg.addIndex((*i)->GetPoint(2)->idx, color);*/

        //_fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(0)->x, (*i)->GetPoint(0)->y, 0 ), c ) );
        //_fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(1)->x, (*i)->GetPoint(1)->y, 0 ), c ) );
        //_fg._vertex_data_triangles.push_back( GeometryEngine::vertex_t( QVector3D( (*i)->GetPoint(2)->x, (*i)->GetPoint(2)->y, 0 ), c ) );
    }

    for ( std::vector<p2t::Point*>::iterator i = polyline.begin(); i != polyline.end(); ++i ) {
        delete *i;
    }

    //delete cdt;
}


OutlineNormals::iterator FluidTriangles::findFirstNormal( unsigned int idx_normal, unsigned int idx_row ) {

    OutlineNormals::iterator inormal = _fg._outline_normals.begin() + idx_normal;

    while ( inormal != _fg._outline_normals.end() && idx_row >= inormal->indicies.size() ) {
        inormal++;
        continue;
    }
    return inormal;
}

int FluidTriangles::getOffsetFromEnd(int idx_row, OutlineNormals::iterator inormal) {
    return inormal->indicies.size() - idx_row - 1;
}

void FluidTriangles::addToContour(unsigned int idx_row, int idx_normal, contour_idxs_t& contour_idxs) {
    GLushort idx_vertex;

    OutlineNormals::iterator inormal = _fg.getOutlineNormal(idx_normal);
    if ( inormal != _fg._outline_normals.end() ) {
        if ( !inormal->indicies.empty() ) {
            if (idx_row >= inormal->indicies.size()) {
                idx_vertex = inormal->indicies.back();
            } else {
                idx_vertex = inormal->indicies[idx_row];
            }

            if ( !contour_idxs.empty() && contour_idxs.back() == idx_vertex ) {
                return;
            }
             contour_idxs.push_back( idx_vertex );
        }
    }
}

FluidTriangles::SCR FluidTriangles::findContour( unsigned int idx_row_0,
                                                 OutlineNormals::iterator inormal_0,
                                                 contour_idxs_t& contour_idxs,
                                                 OutlineNormals::iterator& inormal_next ) {

    typedef std::vector<Edges::edge_link_t*> edges_links_t;
    edges_links_t edge_links;

    auto processBottom = [this, idx_row_0, &inormal_next, &edge_links] ( OutlineNormals::iterator inormal_0,
                                                            OutlineNormals::iterator& inormal_1,
                                                            int& idx_row_1,
                                                            contour_idxs_t& contour_idxs ) -> SCR {
        addToContour( idx_row_0, inormal_0->idx, contour_idxs );

        if ( idx_row_0 == 0 ) {
            inormal_next = inormal_1 = inc_cycled( _fg._outline_normals.begin(), _fg._outline_normals.end(), inormal_0 );
            idx_row_1 = 0;
            addToContour( idx_row_0, inormal_1->idx, contour_idxs );
        } else {
            Edges::edge_link_t * edge_link;
            int idx_row = idx_row_0;
            int idx_normal = inormal_0->idx;

            edge_links.clear();

            while ( _edges.getEdge( inormal_0->idx, idx_normal, idx_row, edge_link ) )  {
                inormal_1 = _fg.getOutlineNormal( edge_link->next->idx_normal );
                idx_row_1 = edge_link->next->idx_row;
                addToContour( idx_row_1, inormal_1->idx, contour_idxs );

                if ( std::find( edge_links.begin(), edge_links.end(), edge_link  ) == edge_links.end() ) {
                    edge_links.push_back( edge_link );
                } else {
                    break;
                }

                idx_row = idx_row_1;
                idx_normal = inormal_1->idx;
            }

            if ( edge_links.empty() ) {
                return SEARCH;
            }

            if ( getOffsetFromEnd( idx_row_0, inormal_0 ) <= 0 &&
                 getOffsetFromEnd( idx_row_1, inormal_1 ) <= 0 ) {
                return SEARCH;
            } else {
                for ( auto i : edge_links ) {
                    _edges.markEdge( *i );
                }
            }
        }
        return SUCCESS;
    };

    auto processSide = [this] ( OutlineNormals::iterator inormal,
                                           int idx_row,
                                           contour_idxs_t& contour_idxs ) -> bool {
        if ( getOffsetFromEnd( idx_row, inormal ) <= 0 ) {
            return true;
        } else {
            OutlineNormals::iterator inormal_branch = _fg._outline_normals.end();
            OutlineNormalLinks::const_iterator ilink = inormal->links_rad.begin();
            for (; ilink != inormal->links_rad.end(); ++ilink ) {
                if ( ilink->type == LINK_RAD_BRANCH && ilink->idx_row <= idx_row ) {
                    OutlineNormals::iterator inormal_linked = _fg.getOutlineNormal( ilink->idx_normal );
                    if ( inormal != _fg._outline_normals.end() ) {
                        QVector3D cross = QVector3D::crossProduct( inormal->vertex_end - inormal->vertex_begin,
                                                                   inormal_linked->vertex_end - inormal_linked->vertex_begin );

                        if ( cross.z() > 0 ) {
                            inormal_branch = inormal_linked;
                        }
                    }
                }
            }

            if ( inormal_branch == _fg._outline_normals.end() ) {
                addToContour( idx_row + 1, inormal->idx, contour_idxs );
            } else {
                addToContour( -1, inormal_branch->idx, contour_idxs );
            }
        }
        return true;
    };

    auto processTop = [this, idx_row_0, inormal_0] ( OutlineNormals::iterator inormal,
                                                                                         int idx_row,
                                                                                         contour_idxs_t& contour_idxs ) -> bool {
            int idx_normal_prev = inormal->idx;
            int idx_row_prev = ( (unsigned)idx_row + 1 >= inormal->indicies.size() ) ? inormal->indicies.size() - 1 : idx_row + 1;

            for (OutlineNormalLinks::const_reverse_iterator ilink = inormal->links_tan_r.rbegin(); ilink != inormal->links_tan_r.rend(); ++ilink) {
                if ( ilink->type == LINK_TAN && ilink->idx_row <= idx_row  ) {
                    if ( ilink->idx_normal != inormal_0->idx &&
                         ilink->idx_normal != idx_normal_prev  ) {
                        addToContour( -1, ilink->idx_normal, contour_idxs );
                        _edges.setEdge( inormal_0->idx, ilink->idx_normal, -1, idx_normal_prev, idx_row_prev );
                        idx_normal_prev = ilink->idx_normal;
                        idx_row_prev = -1;
                     }
                }
            }

            _edges.setEdge( inormal_0->idx, inormal_0->idx, idx_row_0 + 1, idx_normal_prev, idx_row_prev );

            return true;
        };

    if ( inormal_0->indicies.empty() ) {
        return SCR::ERROR;
    }

    contour_idxs.clear();

    OutlineNormals::iterator inormal_1;
    int idx_row;

    SCR br = processBottom( inormal_0, inormal_1, idx_row, contour_idxs );
    if ( br == SCR::SUCCESS ) {
        processSide( inormal_1, idx_row, contour_idxs );
        processTop( inormal_1, idx_row, contour_idxs );
        processSide( inormal_0, idx_row_0, contour_idxs );
    } else {
        return br;
    }

    if ( !_edges.isEmpty( inormal_0->idx,  inormal_0->idx, idx_row_0 ) ) {
        inormal_next = inormal_0;
        return SCR::REPEAT;
    } else {
        inormal_next = inormal_1;
        return br;
    }
}

