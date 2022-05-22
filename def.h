#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include "mesh/def.h"
#include "../vec/vec4.h"
#include "../vec/object3.h"
#endif

typedef struct {
    vec_object3 * origin;
    range_phys_mesh_node mesh;
    window_phys_mesh_tri_p subset;
}
    phys_object;

void phys_object_clear (phys_object * target);
