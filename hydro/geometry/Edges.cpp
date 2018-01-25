#include "Edges.h"/

Edges::Edges( int length ) : _idx_vertex_0(0), _edges( length )
{

}

void Edges::setEdge( int idx_normal_base, int idx_vertex_0, int idx_vertex_1, int idx_normal_1, int idx_row_1, bool is_tan ) {
    list_edge_data_ptr_t edge_data;

    if ( idx_vertex_0 == idx_vertex_1 ) {
        return;
    }

    if ( _edges[idx_vertex_0] ==  nullptr ) {
        edge_data = std::make_shared<list_edge_data_t>();
        _edges[idx_vertex_0] = edge_data;
    } else {
        edge_data = _edges[idx_vertex_0];
    }

    list_edge_data_t::iterator i = std::find_if( edge_data->begin(),
                                                                       edge_data->end(),
                                                                        [idx_vertex_1] ( const edge_data_t& edge ) { return //edge.idx_normal_base == idx_normal_base  &&
                                                                                                                                                             edge.idx_vertex == idx_vertex_1; } );

    if ( i == edge_data->end() ) {
        edge_data->push_back( edge_data_t(idx_normal_base, idx_vertex_1, idx_normal_1, idx_row_1, is_tan) );
    } else {
        i->is_marked = false;
        i->idx_row = idx_row_1;
    }
}

bool Edges::getEdge( int idx_normal_base, int idx_vertex_0, list_edge_data_t::iterator& iedge ) {
    list_edge_data_ptr_t edge_data;

    if ( _edges[idx_vertex_0] ==  nullptr ) {
        return false;
    } else {
        edge_data = _edges[idx_vertex_0];
    }

    if ( !edge_data->empty() ) {
        for ( list_edge_data_t::iterator i = edge_data->begin(); i != edge_data->end(); ++i ) {
            if ( idx_normal_base == i->idx_normal_base && i->is_marked == false ) {
                iedge = i;
                return true;
            }
        }
    }

    return false;
}

void Edges::delEdge( list_edge_data_t::iterator iedge ) {
    iedge->is_marked = true;
}

bool Edges::isEmpty( int idx_normal_base, int idx_vertex ) {
    if ( _edges[idx_vertex] ==  nullptr ) {
        return true;
    } else if ( _edges[idx_vertex]->empty() ) {
        return true;
    } else {
        list_edge_data_ptr_t edge_data = _edges[idx_vertex];
        int c = 0;
        for ( list_edge_data_t::iterator i = edge_data->begin(); i != edge_data->end(); ++i ) {
            if ( idx_normal_base == i->idx_normal_base &&  i->is_marked == false ) {
                c++;
            }
        }
        return c == 0;
    }
}
