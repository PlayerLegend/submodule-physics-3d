#ifndef FLAT_INCLUDES
#include "type.h"
#include "../../range/def.h"
#include "../../vec/vec4.h"
#include "../../vec/object3.h"
#include "../mesh/node/type.h"
#include "../mesh/boundary/type.h"
#include "../mesh/type.h"
#include "../object/type.h"
#endif

void phys_sweep_ellipsoid_against_mesh (phys_sweep_result * result, const phys_object_mesh * world, const phys_object_ellipsoid * ellipsoid, const fvec3 * direction, fvec distance);
