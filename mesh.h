#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stddef.h>
#include "def.h"
#include "../gltf/def.h"
#endif

bool phys_mesh_load (range_phys_mesh_node * result, const glb * glb);
void phys_mesh_sweep_point (phys_mesh_sweep * sweep);
void phys_mesh_sweep_aligned_ellipsoid (phys_mesh_sweep * sweep, fvec3_aligned_ellipsoid * shape);
