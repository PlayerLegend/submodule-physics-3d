#ifndef FLAT_INCLUDES
#include "def.h"
#endif

inline static phys_sweep phys_sweep_init (const fvec3_line * sweep)
{
    return (phys_sweep)
    {
	.path = *sweep,
    };
}

inline static bool phys_sweep_update (phys_sweep * sweep, const fvec3 * hit_normal, fvec hit_distance)
{
    if (hit_distance < 0 || sweep->path.distance < hit_distance)
    {
	return false;
    }
    else
    {
	sweep->is_hit = true;
	sweep->path.distance = hit_distance;
	sweep->hit_normal = *hit_normal;
	
	return true;
    }
}

inline static bool phys_sweep_update_no_normal (phys_sweep * sweep, fvec hit_distance)
{
    if (hit_distance < 0 || sweep->path.distance < hit_distance)
    {
	return false;
    }
    else
    {
	sweep->is_hit = true;
	sweep->path.distance = hit_distance;
	
	return true;
    }
}
