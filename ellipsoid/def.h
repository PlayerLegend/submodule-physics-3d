#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#endif

typedef struct {
    fvec3 semi_axis;
}
    phys_ellipsoid_properties;

fvec phys_ellipsoid_surface_distance (phys_ellipsoid_properties * properties, fvec3 * normal);
