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

FluidTriangles::FluidTriangles(FluidGeometry& fluid_geometry) : _fg(fluid_geometry),
                                                                _path_data(_fg._vertex_data.size())
{
    _color_generator = std::make_shared<ColorGenerator>();
}

void FluidTriangles::triangulate() {

    for (std::vector<path_data_t>::iterator idata = _path_data.begin(); idata != _path_data.end(); ++idata ) {
        idata->edges.clear();
    }

    unsigned int idx_max_row = _fg._idx_max_row;
    contours_idxs_t contours_idxs;

    for (unsigned int idx_row = 0; idx_row < idx_max_row; idx_row++) {
        OutlineNormals::iterator inormal;
        OutlineNormals::iterator inormal_0 = inormal = findFirstNormal(idx_row);

        if ( inormal != _fg._outline_normals.end() ) {
            sectors_t sectors;

            do {
                inormal = findNextSectors( idx_row, inormal, sectors );

                for ( sectors_t::const_iterator isector = sectors.begin(); isector != sectors.end(); ++isector ) {
                    processSector( idx_row, *isector );
                }
            } while ( inormal != inormal_0 && inormal != _fg._outline_normals.end() );
        }
    }


    for ( GeometryEngine::vertex_data_t::iterator idata = _fg._vertex_data.begin(); idata != _fg._vertex_data.end(); ++idata ) {
        idata->normal.normalize();

        QVector3D normal = idata->normal / 30;
        _fg.drawLine( idata->vertex, idata->vertex + normal, QColor(0, 0, 255, 255) );
    }

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

            edges_t& edges_curr = _path_data[*iidx_curr].edges;

            if (std::find( edges_curr.begin(), edges_curr.end(), *iidx_prev) == edges_curr.end() ) {
                edges_curr.push_back( *iidx_prev );
            }
            if (std::find( edges_curr.begin(), edges_curr.end(), *iidx_next) == edges_curr.end() ) {
                edges_curr.push_back( *iidx_next );
            }

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


OutlineNormals::iterator FluidTriangles::findFirstNormal(unsigned int idx_row) {

    OutlineNormals::iterator inormal = _fg._outline_normals.begin();

    while (inormal != _fg._outline_normals.end() && idx_row >= inormal->indicies.size() - 1) {
        inormal++;
        continue;
    }
    return inormal;
}

OutlineNormals::iterator FluidTriangles:: findNextSectors(unsigned int idx_row, OutlineNormals::iterator inormal_0, sectors_t& sectors) {

    auto findSector = [this] ( unsigned int idx_row,
                               OutlineNormals::iterator inormal_0,
                               bool cw, sector_t& sector, bool& is_splitted ) -> bool {
        OutlineNormals::iterator splitter = _fg._outline_normals.end();

        std::function<OutlineNormals::iterator(OutlineNormals::iterator,
                                               OutlineNormals::iterator,
                                               OutlineNormals::iterator)> next;

        if ( cw ) {
            next = inc_cycled<OutlineNormals::iterator>;
        } else {
            next = dec_cycled<OutlineNormals::iterator>;
        }

        OutlineNormals::iterator inormal = inormal_0;

        do {
            inormal = next( _fg._outline_normals.begin(), _fg._outline_normals.end(), inormal );

            if ( splitter != _fg._outline_normals.end() ) {
                if ( FluidGeometryUtils::isSectorsSplitted( _fg._outline_normals, inormal_0, inormal, splitter ) ) {
                    is_splitted = true;
                    if ( cw ) {
                        sector.inormal_r = inormal_0;
                        sector.inormal_l = splitter;
                    } else {
                        sector.inormal_r = splitter;
                        sector.inormal_l = inormal_0;
                    }
                    return true;
                } else {
                    splitter = inormal;
                }
            } else {
                splitter = inormal;
            }

        } while ( idx_row >= inormal->indicies.size() - 1 && inormal != inormal_0 );

        if ( inormal == inormal_0 ) {
            return false;
        } else {
            is_splitted = false;
            sector.inormal_r = inormal_0;
            sector.inormal_l = inormal;
            return true;
        }

    };

    OutlineNormals::iterator inormal = inormal_0;
    do {
        inormal = inc_cycled( _fg._outline_normals.begin(), _fg._outline_normals.end(), inormal );
    } while  ( idx_row >= inormal->indicies.size() - 1 && inormal != inormal_0 );

    sector_t sector(_fg._outline_normals.end(), _fg._outline_normals.end());
    sectors.clear();
    bool is_splitted;

    if ( inormal == inormal_0 ) {
        if ( findSector( idx_row, inormal_0, true, sector, is_splitted ) ) {
            sectors.push_back( sector );
            if ( findSector( idx_row, inormal_0, false, sector, is_splitted ) ) {
                sectors.push_back( sector );
            } else {
                Game::instance().getWorldController()->enableUpdates(false);
            }
        } else {
            Game::instance().getWorldController()->enableUpdates(false);
        }
        return _fg._outline_normals.end();
    } else {
        if ( findSector( idx_row, inormal_0, true, sector, is_splitted ) ) {
            sectors.push_back( sector );
            if ( is_splitted ) {
                if ( findSector( idx_row, inormal, false, sector, is_splitted ) ) {
                    sectors.push_back( sector );
                }
            }
        }
        return inormal;
    }
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

