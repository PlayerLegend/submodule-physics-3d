#include "moving-ellipsoid-against-mesh.h"
#include "../../window/def.h"
#include "../../window/alloc.h"
#include "../../log/log.h"
#include "start.h"

range_typedef(phys_mesh_node*, phys_mesh_node_p);
window_typedef(phys_mesh_node*, const_phys_mesh_node_p);

typedef struct {
    bool hit;
    fvec3_combined_line path;
    fvec3 hit_position;
    const fvec3_triangle * hit_tri;
}
    sphere_sweep;

inline static void sweep_edge (sphere_sweep * result, const fvec3 * v1, const fvec3 * v2)
{
    fvec3_ray_intersect intersect;

    intersect.path = &result->path.ray;

    fvec3 cylinder_delta = { vec3_subtract_init(*v2, *v1) };
    
    fvec cylinder_length = fvec3_vlen(&cylinder_delta);

    fvec cylinder_invlength = 1.0 / cylinder_length;

    fvec3_ray cylinder_axis = { .begin = *v1, .direction = { vec3_scale_init(cylinder_delta, cylinder_invlength) } };

    if (fvec3_ray_cylinder_intersect_distance(&intersect, &cylinder_axis, 1), !intersect.hit)
    {
	return;
    }

    if (intersect.distance >= result->path.distance)
    {
	return;
    }

    fvec3 end = { vec3_ray_point(result->path.ray, intersect.distance) };

    fvec hit_height = vec3_project_distance(cylinder_axis, end);

    if (hit_height < 0 || hit_height > cylinder_length)
    {
	return;
    }

    result->hit = true;
    result->hit_position = (fvec3){ vec3_ray_point(cylinder_axis, hit_height) };
    result->path.distance = intersect.distance;
    result->path.end = end;
    result->hit_tri = NULL;
}

inline static void sweep_vertex (sphere_sweep * result, const fvec3 * vert)
{
    fvec3_ray_intersect intersect;
    intersect.path = &result->path.ray;

    if (fvec3_ray_sphere_intersect_distance(&intersect, vert, 1), !intersect.hit)
    {
	return;
    }

    if (intersect.distance >= result->path.distance)
    {
	return;
    }

    result->hit = true;
    result->hit_position = *vert;
    fvec3_combined_line_update_distance(&result->path, intersect.distance);
    result->hit_tri = NULL;
}

void sweep_nodes (sphere_sweep * result, const phys_object_ellipsoid * ellipsoid, const phys_object_mesh * mesh)
{
    if (range_is_empty(mesh->hull.nodes))
    {
	return;
    }
    
    window_const_phys_mesh_node_p jobs = {0};

    window_alloc(jobs, range_count(mesh->hull.nodes));

    assert (range_is_empty(jobs.region));

    *window_push(jobs) = mesh->hull.nodes.begin;

    const phys_mesh_node * node;

    fvec begin_plane_distance;
    fvec end_plane_distance;
    
    fvec3_ray_intersect intersect = { .path = &result->path.ray };

    size_t count = 0;

    fvec3_triangle sphere_space_triangle;
    fvec3_triangle point_space_triangle;

    while (!range_is_empty(jobs.region))
    {
	count++;
	
	node = range_pop_end(jobs.region);

#define add_job(name) \
	if (node->plane.name)\
	    *window_push(jobs) = node->plane.name;

	add_job(intersect);

	fvec3_point_transfer_object_space(sphere_space_triangle.vertex, ellipsoid->origin, mesh->origin, node->tri.vertex);
	fvec3_point_transfer_object_space(sphere_space_triangle.vertex + 1, ellipsoid->origin, mesh->origin, node->tri.vertex + 1);
	fvec3_point_transfer_object_space(sphere_space_triangle.vertex + 2, ellipsoid->origin, mesh->origin, node->tri.vertex + 2);
        
	vec3_unscale3(sphere_space_triangle.vertex[0], ellipsoid->radius);
	vec3_unscale3(sphere_space_triangle.vertex[1], ellipsoid->radius);
	vec3_unscale3(sphere_space_triangle.vertex[2], ellipsoid->radius);

	fvec3_triangle_set_normal(&sphere_space_triangle);

	point_space_triangle.vertex[0] = (fvec3){ vec3_add_init(sphere_space_triangle.vertex[0], sphere_space_triangle.normal) };
	point_space_triangle.vertex[1] = (fvec3){ vec3_add_init(sphere_space_triangle.vertex[1], sphere_space_triangle.normal) };
	point_space_triangle.vertex[2] = (fvec3){ vec3_add_init(sphere_space_triangle.vertex[2], sphere_space_triangle.normal) };

	point_space_triangle.normal = sphere_space_triangle.normal;

	if (fvec3_ray_triangle_intersect_distance(&intersect, &point_space_triangle), intersect.hit)
	{
	    if (result->path.distance > intersect.distance)
	    {
		result->hit = true;
		fvec3_combined_line_update_distance(&result->path, intersect.distance);
		result->hit_position = (fvec3){ vec3_subtract_init(result->path.end, point_space_triangle.normal) };
		result->hit_tri = &node->tri;
	    }
	    
	    add_job(positive);
	}
	else
	{
	    begin_plane_distance = vec3_plane_distance(sphere_space_triangle, result->path.begin);
	    end_plane_distance = vec3_plane_distance(sphere_space_triangle, result->path.end);

	    if (begin_plane_distance > 1 && end_plane_distance > 1)
	    {
		add_job(positive);
	    }
	    else if (begin_plane_distance < -1 && end_plane_distance < -1)
	    {
		add_job(negative);
	    }
	    else
	    {
		add_job(positive);
		add_job(negative);

		sweep_edge(result, sphere_space_triangle.vertex, sphere_space_triangle.vertex + 1);
		sweep_edge(result, sphere_space_triangle.vertex + 1, sphere_space_triangle.vertex + 2);
		sweep_edge(result, sphere_space_triangle.vertex + 2, sphere_space_triangle.vertex);

		sweep_vertex(result, sphere_space_triangle.vertex);
		sweep_vertex(result, sphere_space_triangle.vertex + 1);
		sweep_vertex(result, sphere_space_triangle.vertex + 2);
	    }
	}
    }
    
    //log_debug("Percent usage: %f", (float) count / (float)range_count(mesh->nodes));

    window_clear(jobs);
}



void phys_sweep_ellipsoid_against_mesh (phys_sweep_result * result, const phys_object_mesh * world, const phys_object_ellipsoid * ellipsoid, const fvec3 * direction, fvec distance)
{
    phys_sweep_result_start(result, ellipsoid->origin, direction, distance);
    
    sphere_sweep ellipsoid_space_sweep = {0};
    
    fvec3_combined_line_to_object_space(&ellipsoid_space_sweep.path, ellipsoid->origin, &result->path);

    ellipsoid_space_sweep.hit_position = ellipsoid_space_sweep.path.end;

    sweep_nodes(&ellipsoid_space_sweep, ellipsoid, world);

    assert (fvec3_vlen(&ellipsoid_space_sweep.path.begin) < FVEC_EPSILON);

    if (!ellipsoid_space_sweep.hit)
    {
	return;
    }

    result->hit = true;

    fvec3 end;
    
    fvec3_point_to_world_space(&end, ellipsoid->origin, &ellipsoid_space_sweep.path.end);

    fvec3_combined_line_update_end(&result->path, &end);
    
    fvec3_point_to_world_space(&result->hit_position, ellipsoid->origin, &ellipsoid_space_sweep.hit_position);

    fvec3 endpoint_space_hit_position = { vec3_subtract_init(ellipsoid_space_sweep.hit_position, ellipsoid_space_sweep.path.end) };

    fvec3 total_scale = ellipsoid->radius;
    vec3_scale3(total_scale, ellipsoid->origin->scale);

    fvec3 ellipsoid_normal = endpoint_space_hit_position;
    
    vec3_unscale3(ellipsoid_normal, total_scale);
    //vec3_unscale3(ellipsoid_normal, total_scale);
    
    fvec3_normalize(&ellipsoid_normal, &ellipsoid_normal);

    vec3_scale(ellipsoid_normal, -1);
    
    fvec4_quaternion_rotate(&result->hit_normal, &ellipsoid->origin->quaternion, &ellipsoid_normal);

    #ifndef NDEBUG
    if (ellipsoid_space_sweep.hit_tri)
    {
	fvec3_triangle world_space_tri;
	fvec3_triangle_to_world_space(&world_space_tri, world->origin, ellipsoid_space_sweep.hit_tri);
	fvec dot = vec3_dot(world_space_tri.normal, result->hit_normal);
	assert (dot > 1 - FVEC_EPSILON);
	assert (dot < 1 + FVEC_EPSILON);
    }
    #endif
}
