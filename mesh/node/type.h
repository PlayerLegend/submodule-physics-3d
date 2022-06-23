#ifndef FLAT_INCLUDES
#include "../../../vec/object3.h"
#include "../../../range/def.h"
#endif

typedef struct phys_mesh_node phys_mesh_node;

struct phys_mesh_node {
    fvec3_triangle tri;
    union {
	phys_mesh_node * is_positive_index[3];
	struct {
	    phys_mesh_node * negative;
	    phys_mesh_node * positive;
	    phys_mesh_node * intersect;
	};
    }
	plane;
};

range_typedef(phys_mesh_node, phys_mesh_node);
