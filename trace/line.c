#include <stdbool.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "def.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../mesh/def.h"
#include "line.h"
#include "../../log/log.h"

void phys_trace_line_tri (phys_trace_result * result, const phys_mesh_tri * tri, const phys_trace_arg * arg)
{
    // Moller-Trumbore algorithm - backface culling version

    fvec3 direction_hack = vec3_scale_init (arg->direction, -1);
    fvec3 origin_hack = vec3_scale_init (arg->origin, -1);
    
    fvec3 edge1 = vec3_subtract_init (tri->position[1], tri->position[0]);
    fvec3 edge2 = vec3_subtract_init (tri->position[2], tri->position[0]);
    //fvec3 edge1 = vec3_subtract_init (tri->position[0], tri->position[1]);
    //fvec3 edge2 = vec3_subtract_init (tri->position[0], tri->position[2]);
    fvec3 p = vec3_cross(direction_hack, edge2);
    fvec determinate = vec3_dot (edge1, p);

    if (determinate < FVEC_EPSILON)
    {
	goto miss;
    }
    
    fvec3 tvec = vec3_subtract_init(origin_hack, tri->position[0]);
    //fvec3 tvec = vec3_subtract_init(origin_hack, plane_point);

    fvec u = vec3_dot (tvec, p);

    if (u < 0.0 || u > determinate)
    {
	goto miss;
    }

    fvec3 q = vec3_cross (tvec, edge1);

    fvec v = vec3_dot (direction_hack, q);

    if (v < 0.0 || u + v > determinate)
    {
	goto miss;
    }

    fvec t = vec3_dot (edge2, q) / determinate;

    if (t < 0)
    {
	goto miss;
    }

    *result = (phys_trace_result)
    {
	.is_hit = true,
	.distance = t,
	.hit_normal = tri->normal,
	.end = vec3_line_point_init(origin_hack, direction_hack, t),
    };

    //fvec3 test_vector = vec3_subtract_init (tri->position[0], result->end);

    //log_debug("test dot %f", vec3_dot (test_vector, tri->normal));
    
    return;

miss:
    *result = (phys_trace_result)
	{
	    .is_hit = false,
	};
    return;
}

void phys_trace_line_mesh_subset (phys_trace_result * result, const range_const_phys_mesh_tri_p * set, const phys_trace_arg * arg)
{
    *result = (phys_trace_result)
    {
	.is_hit = false,
	.distance = arg->distance,
    };
    
    const phys_mesh_tri ** tri;
    phys_trace_result tri_result;

    /*phys_trace_arg arg_hack = *arg;

    arg_hack.direction.x *= -1;
    arg_hack.direction.y *= -1;
    arg_hack.direction.z *= -1;*/
    //log_debug("here");
    //phys_mesh_tri hack;
    for_range (tri, *set)
    {
	/*hack = **tri;
	hack.position[0].x *= -1;
	hack.position[0].y *= -1;
	hack.position[0].z *= -1;
	hack.position[1].x *= -1;
	hack.position[1].y *= -1;
	hack.position[1].z *= -1;
	hack.position[0].x *= -1;
	hack.position[0].y *= -1;
	hack.position[0].z *= -1;*/
	
	phys_trace_line_tri(&tri_result, *tri, arg);

	if (!tri_result.is_hit)
	{
	    continue;
	}

	if (result->distance > tri_result.distance)
	{
	    *result = tri_result;
	}
    }
    //log_debug("hit %d dist %f end %f %f %f, origin %f %f %f", result->is_hit, result->distance, result->end.x, result->end.y, result->end.z, arg->origin.x, arg->origin.x, arg->origin.y, arg->origin.z);
}
