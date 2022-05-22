#include "line.h"
#include "../mesh/subset.h"
#include "../../log/log.h"
#include "transform.h"

void phys_trace_line_tri (phys_trace_result * result, const phys_mesh_tri * tri, const vec_trace3 * trace)
{
    // Moller-Trumbore algorithm - backface culling version

    //fvec3 direction_hack = vec3_scale_init (trace->direction, 1);
    //fvec3 origin_hack = vec3_scale_init (trace->position, 1);
    
    fvec3 edge1 = vec3_subtract_init (tri->position[1], tri->position[0]);
    fvec3 edge2 = vec3_subtract_init (tri->position[2], tri->position[0]);
    //fvec3 edge1 = vec3_subtract_init (tri->position[0], tri->position[1]);
    //fvec3 edge2 = vec3_subtract_init (tri->position[0], tri->position[2]);
    fvec3 p = vec3_cross(trace->direction, edge2);
    fvec determinate = vec3_dot (edge1, p);

    if (determinate < FVEC_EPSILON)
    {
	goto miss;
    }
    
    fvec3 tvec = vec3_subtract_init(trace->position, tri->position[0]);
    //fvec3 tvec = vec3_subtract_init(origin_hack, plane_point);

    fvec u = vec3_dot (tvec, p);

    if (u < 0.0 || u > determinate)
    {
	goto miss;
    }
    
    fvec3 q = vec3_cross (tvec, edge1);

    fvec v = vec3_dot (trace->direction, q);

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
	.end = vec3_line_point_init(trace->position, trace->direction, t),
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

void phys_trace_line_mesh_subset (phys_trace_result * result, const range_const_phys_mesh_tri_p * set, const vec_trace3 * trace)
{
    *result = phys_trace_result_init(*trace);

    const phys_mesh_tri ** tri;
    phys_trace_result tri_result;

    for_range (tri, *set)
    {
	phys_trace_line_tri(&tri_result, *tri, trace);

	if (!tri_result.is_hit)
	{
	    continue;
	}

	if (result->distance > tri_result.distance)
	{
	    *result = tri_result;
	}
    }
}

void phys_trace_line_object (phys_trace_result * result, phys_object * object, const vec_trace3 * trace)
{
    vec_trace3 trace_transform;

    phys_trace_transform(&trace_transform, object->origin, trace);
    
    phys_mesh_subset_all(&object->subset, &object->mesh.alias_const); // unoptimized for testing
    
    phys_trace_result result_transform;
    
    phys_trace_line_mesh_subset(&result_transform, &object->subset.region.alias_const, &trace_transform);

    phys_trace_untransform(result, object->origin, &result_transform);
}
