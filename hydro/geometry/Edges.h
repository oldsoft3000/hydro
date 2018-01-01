#ifndef EDGES_H
#define EDGES_H

#include "FluidGeometry.h"
#include <list>
#include <memory>

class Edges
{
private:
    typedef struct edge_data_t {
        int     idx_vertex;
        int     idx_normal;
        int     idx_row;

        edge_data_t( int idx_vertex_, int idx_normal_, int idx_row_ ) : idx_vertex(idx_vertex_), idx_normal(idx_normal_), idx_row(idx_row_) {
        }
    };

    typedef std::list<edge_data_t> list_edge_data_t;
    typedef std::shared_ptr<list_edge_data_t> list_edge_data_ptr_t;

    int                                 _idx_vertex_0;
    std::vector<list_edge_data_ptr_t>   _edges;
public:
    Edges( int length );
    void pushEdge( int idx_vertex_0, int idx_vertex_1, int idx_normal_1, int idx_row_1 );
    bool popEdge( int idx_vertex_0, int& idx_vertex_1, int& idx_normal_1, int& idx_row_1 );
    bool isEmpty( int idx_vertex );
};

typedef std::shared_ptr<Edges> EdgesPtr;

#endif // EDGES_H
