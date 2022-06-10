#ifndef FLAT_INCLUDES
#include "mesh.h"
#include "sweep.h"
#endif

void phys_object_sweep_point (phys_object * object, phys_mesh_sweep * sweep, fvec3_line * path);
void phys_object_sweep_aligned_ellipsoid (phys_object * object, phys_mesh_sweep * sweep, fvec3_line * path, fvec3_aligned_ellipsoid * shape);
