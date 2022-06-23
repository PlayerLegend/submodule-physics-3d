#include "moving-point-against-mesh.h"
#include "../../window/def.h"
#include "../../window/alloc.h"
#include "../../log/log.h"
#include "start.h"

range_typedef(phys_mesh_node*, phys_mesh_node_p);
window_typedef(phys_mesh_node*, const_phys_mesh_node_p);

typedef struct {
    fvec3_combined_line path;
    const fvec3_triangle * hit;
}
    triangle_sweep;

void sweep_point_against_nodes (triangle_sweep * result, const range_const_phys_mesh_node * nodes)
{
    if (range_is_empty(*nodes))
    {
	return;
    }
    
    window_const_phys_mesh_node_p jobs = {0};

    window_alloc(jobs, range_count(*nodes));

    assert (range_is_empty(jobs.region));

    *window_push(jobs) = nodes->begin;

    const phys_mesh_node * node;

    bool is_begin_positive;
    bool is_end_positive;

    fvec3_ray_intersect intersect = { .path = &result->path.ray };

    size_t count = 0;

    while (!range_is_empty(jobs.region))
    {
	count++;
	
	node = range_pop_end(jobs.region);

#define add_job(name) \
	if (node->plane.name)\
	    *window_push(jobs) = node->plane.name;

	add_job(intersect);

	if (fvec3_ray_triangle_intersect_distance(&intersect, &node->tri), intersect.hit)
	{
	    if (intersect.distance < result->path.distance)
	    {
		fvec3_combined_line_update_distance(&result->path, intersect.distance);
		result->hit = &node->tri;
	    }

	    add_job(positive);
	}
	else
	{
	    is_begin_positive = vec3_plane_distance(node->tri, result->path.begin) >= 0;
	    is_end_positive = vec3_plane_distance(node->tri, result->path.end) >= 0;

	    if (is_begin_positive && is_end_positive)
	    {
		add_job(positive);
	    }
	    else if(!is_begin_positive && !is_end_positive)
	    {
		add_job(negative);
	    }
	    else
	    {
		add_job(positive);
		add_job(negative);
	    }

	    //add_job(positive);
	    //add_job(negative);
	}
    }

    log_debug("Percent usage: %f", (float) count / (float)range_count(*nodes));

    window_clear(jobs);
}

void phys_sweep_point_against_mesh (phys_sweep_result * result, const phys_object_mesh * world, const phys_object_point * point, const fvec3 * direction, fvec distance)
{
    phys_sweep_result_start(result, point->origin, direction, distance);
    
    triangle_sweep mesh_space_sweep;

    mesh_space_sweep.hit = NULL;

    fvec3_combined_line_to_object_space(&mesh_space_sweep.path, world->origin, &result->path);

    fvec3_ray_intersect bbox_intersect;

    bbox_intersect.path = &mesh_space_sweep.path.ray;

    if (fvec3_ray_sphere_intersect_distance(&bbox_intersect, &world->hull.boundary.offset, world->hull.boundary.radius), !bbox_intersect.hit || bbox_intersect.distance > mesh_space_sweep.path.distance)
    {
	return;
    }

    sweep_point_against_nodes (&mesh_space_sweep, &world->hull.nodes.alias_const);

    if (!mesh_space_sweep.hit)
    {
	return;
    }

    fvec3_triangle world_space_triangle;

    fvec3_triangle_to_world_space(&world_space_triangle, world->origin, mesh_space_sweep.hit);

    result->hit = true;
    result->hit_normal = world_space_triangle.normal;
    fvec3_point_to_world_space(&result->hit_position, world->origin, &mesh_space_sweep.path.end);
    fvec3_combined_line_update_end(&result->path, &result->hit_position);
}
