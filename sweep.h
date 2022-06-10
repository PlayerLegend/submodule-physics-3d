#ifndef FLAT_INCLUDES
#include "def.h"
#endif

inline static phys_sweep phys_sweep_init (const fvec3_line * sweep)
{
    return (phys_sweep)
    {
	.path = *sweep,
	.endpoint = { fvec3_ray_point(sweep->ray, sweep->distance) },
    };
}

inline static bool phys_sweep_update_no_normal (phys_sweep * sweep, fvec hit_distance)
{
    if (0 <= hit_distance && hit_distance < sweep->path.distance)
    {
	sweep->is_hit = true;
	sweep->path.distance = hit_distance;
	sweep->endpoint = (fvec3){ fvec3_ray_point(sweep->path.ray, hit_distance) };
	return true;
    }
    else
    {
	return false;
    }
}

inline static bool phys_sweep_update (phys_sweep * sweep, const fvec3 * hit_normal, fvec hit_distance)
{
    if (phys_sweep_update_no_normal(sweep, hit_distance))
    {
	sweep->hit_normal = *hit_normal;
	return true;
    }
    else
    {
	return false;
    }
}
