#include "Edges.h"

Edges::Edges( int length ) : _idx_vertex_0(0), _edges( length )
{

}

void Edges::pushEdge( int idx_vertex_0, int idx_vertex_1, int idx_normal_1, int idx_row_1 ) {
    list_edge_data_ptr_t edge_data;

    if ( _edges[idx_vertex_0] ==  nullptr ) {
        edge_data = std::make_shared<list_edge_data_t>();
        _edges[idx_vertex_0] = edge_data;
    } else {
        edge_data = _edges[idx_vertex_0];
    }

    if ( std::find_if( edge_data->begin(),
                       edge_data->end(),
                       [idx_vertex_1] ( const edge_data_t& edge ) { return edge.idx_vertex == idx_vertex_1; } ) == edge_data->end() ) {
        edge_data->push_back( edge_data_t(idx_vertex_1, idx_normal_1, idx_row_1) );
    }
}

bool Edges::popEdge( int idx_vertex_0, int& idx_vertex_1, int& idx_normal_1, int& idx_row_1 ) {
    list_edge_data_ptr_t edge_data;

    if ( _edges[idx_vertex_0] ==  nullptr ) {
        return false;
    } else {
        edge_data = _edges[idx_vertex_0];
    }

    if ( !edge_data->empty() ) {
        idx_vertex_1 = edge_data->back().idx_vertex;
        idx_normal_1 = edge_data->back().idx_normal;
        idx_row_1 = edge_data->back().idx_row;
        edge_data->pop_back();
        return true;
    } else {
        return false;
    }
}

bool Edges::isEmpty( int idx_vertex ) {
    if ( _edges[idx_vertex] ==  nullptr ) {
        return true;
    } else if ( _edges[idx_vertex]->empty() ) {
        return true;
    } else {
        return false;
    }
}
