#include <stddef.h>
#include <math.h>
#include "../../../glad/include/glad/glad.h"
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../../gltf/accessor_loaders/GLfloat3.h"
#include "def.h"

void phys_mesh_tri_from_GLfloat3 (phys_mesh_tri * result, GLfloat3 positions[3])
{
    fvec3 a = vec3_subtract_init(positions[0], positions[1]);
    fvec3 b = vec3_subtract_init(positions[0], positions[2]);

    fvec3 cross = vec3_cross (a, b);

    fvec cross_scale = 1.0 / sqrt (cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);

    vec3_scale (cross, cross_scale);

    result->position[0] = (fvec3) vec3_initializer (positions[0]);
    result->position[1] = (fvec3) vec3_initializer (positions[1]);
    result->position[2] = (fvec3) vec3_initializer (positions[2]);
    
    result->normal = cross;
}
