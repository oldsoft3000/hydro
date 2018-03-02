#include "Edges.h"

Edges::Edges( FluidGeometry& fg ) {
    _nodes.resize( fg._outline_normals.size() );
    int n = 0;
    for ( auto normal : fg._outline_normals ) {
        _nodes[n].resize( normal.indicies.size() );
        n++;
    }
}

void Edges::setEdge( int idx_normal_base, int idx_normal, int idx_row, int idx_normal_linked, int idx_row_linked ) {
    if ( (unsigned)idx_row >=_nodes[idx_normal] .size() ) {
        idx_row = _nodes[idx_normal] .size() - 1;
    }
    if ( (unsigned)idx_row_linked >=_nodes[idx_normal_linked] .size() ) {
        idx_row_linked = _nodes[idx_normal_linked] .size() - 1;
    }

    edge_node_ptr_t edge_node = getCreateEdge( idx_normal, idx_row );

    std::vector<edge_link_t>::iterator i = std::find_if( edge_node->links.begin(),
                                                                            edge_node->links.end(),
                                                                            [idx_normal_linked, idx_row_linked, idx_normal_base] ( const edge_link_t& link ) {
                                                                                return link.next->idx_normal ==idx_normal_linked  &&
                                                                                            link.next->idx_row == idx_row_linked &&
                                                                                            link.idx_normal_base == idx_normal_base; } );

    if ( i == edge_node->links.end() ) {
        edge_node_ptr_t edge_node_linked = getCreateEdge( idx_normal_linked, idx_row_linked );

        edge_link_t     link;
        link.next = edge_node_linked.get();
        link.marked = false;
        link.idx_normal_base = idx_normal_base;
        edge_node->links.push_back( link );
    }
}

bool Edges::getEdge( int idx_normal_base, int idx_normal, int idx_row, edge_link_t *&edge_link ) {
    if ( (unsigned)idx_row >=_nodes[idx_normal] .size() ) {
        idx_row = _nodes[idx_normal] .size() - 1;
    }

   edge_node_ptr_t  edge_node;

   edge_link = nullptr;

    if ( _nodes[idx_normal][idx_row] ==  nullptr ) {
        return false;
    } else {
        edge_node = _nodes[idx_normal][idx_row];
    }

    if ( !edge_node->links.empty() ) {
        for (  auto i = edge_node->links.begin(); i != edge_node->links.end(); ++i ) {
            if (  i->next && idx_normal_base == i->idx_normal_base && i->marked == false ) {
                edge_link = &*i;
                return true;
            }
        }
    }

    return false;
}

void Edges::markEdge( edge_link_t& edge_link  ) {
    edge_link.marked = true;
}

bool Edges::isEmpty( int idx_normal_base, int idx_normal, int idx_row ) {
    if ( _nodes[idx_normal][idx_row] ==  nullptr ) {
        return true;
    } else if ( _nodes[idx_normal][idx_row]->links.empty() ) {
        return true;
    } else {
        edge_node_ptr_t edge_node = _nodes[idx_normal][idx_row] ;
        int c = 0;
        for (  auto i : edge_node->links ) {
            if ( idx_normal_base == i.idx_normal_base &&  i.marked == false ) {
                c++;
            }
        }
        return c == 0;
    }
}

Edges::edge_node_ptr_t  Edges::getCreateEdge( int idx_normal, int idx_row ) {
    edge_node_ptr_t     edge_node;

    if ( _nodes[idx_normal][idx_row] ==  nullptr ) {
        edge_node = std::make_shared<edge_node_t>();
        edge_node->idx_normal = idx_normal;
        edge_node->idx_row = idx_row;
        _nodes[idx_normal][idx_row] = edge_node;
    } else {
        edge_node = _nodes[idx_normal][idx_row];
    }

    return edge_node;
}
