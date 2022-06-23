#ifndef FLAT_INCLUDES
#include "type.h"
#include <stdint.h>
#include "../../window/def.h"
#include "../../gltf/type.h"
#endif

bool phys_object_set_mesh (phys_object * target, const glb * mesh);

void phys_object_set_point (phys_object * target);

void phys_object_set_ellipsoid (phys_object * target);
