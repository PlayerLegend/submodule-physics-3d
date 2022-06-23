#ifndef FLAT_INCLUDES
#include "../../vec/object3.h"
#include "../../range/def.h"
#include "node/type.h"
#include "boundary/type.h"
#endif

typedef struct {
    range_phys_mesh_node nodes;
    phys_mesh_boundary boundary;
}
    phys_mesh;
