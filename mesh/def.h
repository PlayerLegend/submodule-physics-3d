#ifndef FLAT_INCLUDES
#include <stddef.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#endif

typedef struct {
    fvec3 position[3];
    fvec3 normal;
}
    phys_mesh_tri;

range_typedef(phys_mesh_tri,phys_mesh_tri);
window_typedef(phys_mesh_tri,phys_mesh_tri);
range_typedef(phys_mesh_tri*,phys_mesh_tri_p);
window_typedef(phys_mesh_tri*,phys_mesh_tri_p);

typedef struct phys_mesh_node phys_mesh_node;

struct phys_mesh_node {
    phys_mesh_tri tri;
    size_t plane_cross;
    size_t plane_positive;
    size_t plane_negative;
};

range_typedef(phys_mesh_node,phys_mesh_node);
