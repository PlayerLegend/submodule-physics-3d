#include "set.h"
#include "../../range/alloc.h"
#include "../mesh/load.h"
#include "../mesh/free.h"

inline static void update_min (fvec3 * min, const fvec3 * point)
{
    if (min->x > point->x)
    {
	min->x = point->x;
    }
    
    if (min->y > point->y)
    {
	min->y = point->y;
    }

    if (min->z > point->z)
    {
	min->z = point->z;
    }
}

inline static void update_min_tri (fvec3 * min, const fvec3_triangle * tri)
{
    update_min (min, tri->vertex);
    update_min (min, tri->vertex + 1);
    update_min (min, tri->vertex + 2);
}

inline static void update_max (fvec3 * max, const fvec3 * point)
{
    if (max->x < point->x)
    {
	max->x = point->x;
    }
    
    if (max->y < point->y)
    {
	max->y = point->y;
    }

    if (max->z < point->z)
    {
	max->z = point->z;
    }
}

inline static void update_max_tri (fvec3 * max, const fvec3_triangle * tri)
{
    update_max (max, tri->vertex);
    update_max (max, tri->vertex + 1);
    update_max (max, tri->vertex + 2);
}

inline static void update_radius(fvec * radius, const fvec3 * a, const fvec3 * b)
{
    fvec3 delta = (fvec3){ vec3_subtract_init(*a, *b) };
    
    fvec delta_radius = fvec3_vlen(&delta);

    if (*radius < delta_radius)
    {
	*radius = delta_radius;
    }
}

inline static void clear_existing_hull(phys_object * target)
{
    if (target->type == PHYS_OBJECT_MESH)
    {
	phys_mesh_clear(&target->mesh.hull);
    }
}

bool phys_object_set_mesh (phys_object * target, const glb * mesh)
{
    phys_mesh new_mesh;

    if (!phys_mesh_load(&new_mesh, mesh))
    {
	return false;
    }

    clear_existing_hull(target);
    
    target->mesh.hull = new_mesh;

    target->type = PHYS_OBJECT_MESH;


    return true;
}

void phys_object_set_point (phys_object * target)
{
    clear_existing_hull(target);

    target->type = PHYS_OBJECT_POINT;
}

void phys_object_set_ellipsoid (phys_object * target)
{
    clear_existing_hull(target);

    target->type = PHYS_OBJECT_ELLIPSOID;

    target->ellipsoid.radius = (fvec3){ .x = 1, .y = 1, .z = 1 };
}
