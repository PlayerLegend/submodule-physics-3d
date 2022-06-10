#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../vec/object3.h"
#include "../range/def.h"
#include "../window/def.h"
#endif

typedef struct {
    bool is_hit;
    fvec3 hit_normal;
    fvec3_line path;
}
    phys_sweep;

inline static void phys_sweep_start (phys_sweep * target, fvec3_line * path)
{
    *target = (phys_sweep) { .path = *path };
}

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
window_typedef(phys_mesh_node, phys_mesh_node);

range_typedef(phys_mesh_node*, phys_mesh_node_p);
window_typedef(phys_mesh_node*, phys_mesh_node_p);

typedef struct {
    phys_sweep result;
    window_phys_mesh_node_p todo;
    size_t _total_size;
}
    phys_mesh_sweep;

void phys_mesh_sweep_start (phys_mesh_sweep * target, fvec3_line * path, const range_phys_mesh_node * mesh);

typedef struct {
    vec_object3 * origin;
    range_phys_mesh_node mesh;
}
    phys_object;

void phys_object_clear (phys_object * target);
