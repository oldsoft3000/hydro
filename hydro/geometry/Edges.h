#ifndef EDGES_H
#define EDGES_H

#include "FluidGeometry.h"
#include <vector>
#include <memory>

class Edges
{
public:
    class edge_data_t {
        friend class Edges;
    public:
        int     idx_normal_base;
        int     idx_vertex;
        int     idx_normal;
        int     idx_row;
        bool  is_tan;
    private:
        int     is_marked;
    public:
        edge_data_t( int idx_normal_base_,
                     int idx_vertex_,
                     int idx_normal_,
                     int idx_row_,
                     int is_tan_ ) : idx_normal_base(idx_normal_base_),
                                      idx_vertex(idx_vertex_),
                                      idx_normal(idx_normal_),
                                      idx_row(idx_row_),
                                      is_tan(is_tan_),
                                      is_marked( false )
        {
        }
    };

    typedef std::vector<edge_data_t> list_edge_data_t;
    typedef std::shared_ptr<list_edge_data_t> list_edge_data_ptr_t;
    typedef std::vector<list_edge_data_ptr_t> edges_t;
private:
    int         _idx_vertex_0;
    edges_t     _edges;
public:
    Edges( int length );
    void setEdge( int idx_normal_base, int idx_vertex_0, int idx_vertex_1, int idx_normal_1, int idx_row_1, bool is_tan );
    bool getEdge( int idx_normal_base, int idx_vertex_0, list_edge_data_t::iterator& iedge );
    void delEdge( list_edge_data_t::iterator iedge );
    bool isEmpty( int idx_normal_base, int idx_vertex );
};

typedef std::shared_ptr<Edges> EdgesPtr;

#endif // EDGES_H
