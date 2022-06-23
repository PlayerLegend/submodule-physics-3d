#include "set.h"

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

void phys_mesh_boundary_set(phys_mesh_boundary * boundary, range_phys_mesh_node * nodes)
{
    if (range_is_empty(*nodes))
    {
	*boundary = (phys_mesh_boundary){0};
	return;
    }

    struct {
	fvec3 min;
	fvec3 max;
    }
	bbox = { .min = nodes->begin->tri.point, .max = nodes->begin->tri.point };

    phys_mesh_node * i;

    for_range(i, *nodes)
    {
	update_min_tri(&bbox.min, &i->tri);
	update_max_tri(&bbox.max, &i->tri);
    }

    fvec3 sum = { vec3_add_init(bbox.max, bbox.min) };
    boundary->offset = (fvec3){ vec3_scale_init(sum, 0.5) };

    boundary->radius = 0;

    for_range(i, *nodes)
    {
	update_radius(&boundary->radius, i->tri.vertex, &boundary->offset);
	update_radius(&boundary->radius, i->tri.vertex + 1, &boundary->offset);
	update_radius(&boundary->radius, i->tri.vertex + 2, &boundary->offset);
    }
}
