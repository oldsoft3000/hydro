#ifndef EDGES_H
#define EDGES_H

#include <vector>
#include <memory>

#include "FluidGeometry.h"

class Edges
{
public:
    class edge_node_t;
    struct edge_link_t {
        edge_node_t*    next;
        bool                    marked;
        int                       idx_normal_base;
    };

    struct edge_node_t {

        int     idx_normal;
        int     idx_row;

        edge_node_t() : idx_normal(-1), idx_row(-1) {}

        std::vector<edge_link_t>    links;
    };

    typedef std::shared_ptr<edge_node_t> edge_node_ptr_t;

private:
    std::vector<std::vector<edge_node_ptr_t>>   _nodes;
private:
    edge_node_ptr_t  getCreateEdge( int idx_normal, int idx_row );
public:
    Edges( FluidGeometry& fg );
    void setEdge( int idx_normal_base, int idx_normal, int idx_row, int idx_normal_linked, int idx_row_linked );
    bool getEdge( int idx_normal_base, int idx_normal, int idx_row, edge_link_t *&edge_link );
    void markEdge( edge_link_t& edge_link );
    bool isEmpty( int idx_normal_base, int idx_normal, int idx_row );
};

typedef std::shared_ptr<Edges> EdgesPtr;

#endif // EDGES_H
