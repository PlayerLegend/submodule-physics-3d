#ifndef FLAT_INCLUDES
#include "type.h"
#endif

inline static void phys_sweep_result_start (phys_sweep_result * result, const vec_object3 * object, const fvec3 * direction, fvec distance)
{
    fvec3_ray_line line = { .ray = { .begin = object->position, .direction = *direction }, .distance = distance };

    fvec3_combined_line_from_ray_line(&result->path, &line);

    result->hit_position = result->path.end;
    result->hit_normal = (fvec3){ vec3_negate_init(*direction) };
    result->hit = false;
}

