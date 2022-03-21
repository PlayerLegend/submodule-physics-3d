#include <math.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "def.h"

#define square(n) (fvec)(n * n)

fvec phys_ellipsoid_surface_distance (phys_ellipsoid_properties * properties, fvec3 * normal)
{
    return 1.0 / sqrt ( square(normal->x) / square(properties->semi_axis.x) +
			square(normal->y) / square(properties->semi_axis.y) +
			square(normal->z) / square(properties->semi_axis.z) );
}
