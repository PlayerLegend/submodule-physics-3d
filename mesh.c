#include "mesh.h"
#include <math.h>
#include <assert.h>
#include "../window/alloc.h"
#include "../gltf/env.h"
#include "../log/log.h"
#include "../range/alloc.h"
#include "sweep.h"
#include <stdlib.h>
#include "../vec/vec2.h"

/*static void set_edge (fvec3_line * edge, const fvec3 * vertex, const fvec3 * end)
{
    edge->vertex = *vertex;
    fvec3 delta = vec3_subtract_init(*end, *vertex);
    edge->distance = fvec3_vlen (&delta);
    fvec scale = 1.0 / edge->distance;
    edge->direction = (fvec3) vec3_scale_init(delta, scale);
    }*/

static void fvec3_triangle_from_fvec3 (fvec3_triangle * result, fvec3 positions[3])
{
    fvec3 a = { vec3_subtract_init(positions[0], positions[1]) };
    fvec3 b = { vec3_subtract_init(positions[0], positions[2]) };

    fvec3 cross = { vec3_cross_init (a, b) };

    fvec cross_scale = 1.0 / fvec3_vlen(&cross);

    vec3_scale (cross, cross_scale);

    result->normal = cross;

    result->vertex[0] = positions[0];
    result->vertex[1] = positions[1];
    result->vertex[2] = positions[2];
    
/*    set_edge (result->edge, positions, positions + 1);
    set_edge (result->edge + 1, positions + 1, positions + 2);
    set_edge (result->edge + 2, positions + 2, positions);*/
}

range_typedef(fvec3_triangle, fvec3_triangle);
window_typedef(fvec3_triangle, fvec3_triangle);

typedef struct {
    window_fvec3_triangle * tris;
    fvec3 positions[3];
    int count;
}
    fvec3_triangle_target;

static bool fvec3_triangle_loader(void * target_void, const fvec3 * input)
{
    fvec3_triangle_target * target = target_void;

    target->positions[target->count++] = *input;

    if (target->count == 3)
    {
	target->count = 0;

	fvec3_triangle_from_fvec3(window_push(*target->tris), target->positions);
    }

    return true;
}

static bool fvec3_triangle_load_glb (window_fvec3_triangle * result, const glb * glb)
{
    gltf_accessor_env env;
    gltf_mesh * mesh;
    gltf_mesh_primitive * primitive;
    window_gltf_index indices = {0};
    fvec3_triangle_target target = { .tris = result };
    
    for_range(mesh, glb->gltf.meshes)
    {
	for_range(primitive, mesh->primitives)
	{
	    if (!gltf_accessor_env_setup (&env, &glb->toc, primitive->indices))
	    {
		log_fatal ("Invalid index accessor");
	    }

	    window_rewrite(indices);

	    if (!gltf_accessor_env_load_indices (&indices, &env))
	    {
		log_fatal ("Could not load indicies");
	    }

	    if (!gltf_accessor_env_setup (&env, &glb->toc, primitive->attributes.position))
	    {
		log_fatal ("Invalid position accessor");
	    }
	    
	    if (!gltf_accessor_env_load_fvec3(&target, fvec3_triangle_loader, &indices.region, &env))
	    {
		log_fatal ("Failed to load from positions accessor");
	    }

	    if (target.count != 0)
	    {
		log_fatal ("Vertex count in triangle buffer is not divisible by 3");
	    }
	}
    }

    free (indices.alloc.begin);

    return true;

fail:
    free (indices.alloc.begin);
    return false;
}

range_typedef(fvec3_triangle*,fvec3_triangle_p);
window_typedef(fvec3_triangle*,fvec3_triangle_p);

typedef struct phys_mesh_task phys_mesh_task;

struct phys_mesh_task {
    window_fvec3_triangle_p args;
    phys_mesh_node ** retval;
    phys_mesh_task * pool_next;
};

typedef struct {
    phys_mesh_task * active;
    phys_mesh_task * free;
}
    phys_mesh_task_pool;

static phys_mesh_task * phys_mesh_task_alloc (phys_mesh_task_pool * pool)
{
    if (pool->free)
    {
	phys_mesh_task * retval = pool->free;
	pool->free = retval->pool_next;
	window_rewrite(retval->args);
	return retval;
    }
    else
    {
	return calloc (1, sizeof(phys_mesh_task));
    }
}

inline static void phys_mesh_task_activate (phys_mesh_task_pool * pool, phys_mesh_task * task)
{
    task->pool_next = pool->active;
    pool->active = task;
}

inline static phys_mesh_task * phys_mesh_task_pop (phys_mesh_task_pool * pool)
{
    phys_mesh_task * retval = pool->active;
    if (retval)
    {
	pool->active = retval->pool_next;
    }
    return retval;
}

inline static void phys_mesh_task_free (phys_mesh_task_pool * pool, phys_mesh_task * target)
{
    assert (target != pool->active);
    assert (target != pool->free);
    target->pool_next = pool->free;
    pool->free = target;
    window_rewrite(target->args);
}

static bool is_point_positive (const fvec3_triangle * plane, const fvec3 * point)
{
    fvec3 displacement = { vec3_subtract_init(*point, plane->point) };

    return vec3_dot(displacement, plane->normal) >= 0;
}

static double get_point_plane_distance (const fvec3_triangle * plane, const fvec3 * point)
{
    fvec3 displacement = { vec3_subtract_init(*point, plane->point) };

    fvec dist = vec3_dot(displacement, plane->normal);

    if (dist < 0)
    {
	return -dist;
    }
    else
    {
	return dist;
    }
}

static void find_list_center (fvec3 * center, const range_const_fvec3_triangle_p * list)
{
    *center = (fvec3){0};

    const fvec3_triangle ** i;
    const fvec3_triangle * i_ref;

    for_range(i, *list)
    {
	i_ref = *i;

	vec3_add(*center, i_ref->vertex[0]);
	vec3_add(*center, i_ref->vertex[1]);
	vec3_add(*center, i_ref->vertex[2]);
    }

    fvec scale = 1.0 / (3.0 * (fvec) range_count(*list));

    vec3_scale (*center, scale);
}

static fvec3_triangle * pop_splitting_triangle (range_fvec3_triangle_p * input)
{
    if (range_is_empty (*input))
    {
	return NULL;
    }

    size_t start_size = range_count (*input);
    
    fvec3 list_center;

    find_list_center (&list_center, &input->alias_const);

    fvec3_triangle ** i;

    fvec3_triangle ** choice = input->begin;
    assert (*choice);
    fvec choice_distance = get_point_plane_distance(*choice, &list_center);
    fvec distance;

    for_range (i, *input)
    {
	distance = get_point_plane_distance(*i, &list_center);

	if (distance < choice_distance)
	{
	    choice_distance = distance;
	    choice = i;
	}
    }

    fvec3_triangle * retval = *choice;

    input->end--;

    *choice = *input->end;

    assert ((size_t)range_count(*input) == start_size - 1);

    return retval;
}

static void split_list (window_fvec3_triangle_p * positive, window_fvec3_triangle_p * intersect, window_fvec3_triangle_p * negative, const fvec3_triangle * plane, range_fvec3_triangle_p * input)
{
    fvec3_triangle ** i;
    fvec3_triangle * i_ref;

    bool is_positive;

    for_range (i, *input)
    {
	i_ref = *i;

	is_positive = is_point_positive(plane, &i_ref->vertex[0]);

	if ( is_point_positive(plane, &i_ref->vertex[1]) != is_positive ||
	     is_point_positive(plane, &i_ref->vertex[2]) != is_positive)
	{
	    *window_push(*intersect) = i_ref;
	}
	else if (is_positive)
	{
	    *window_push(*positive) = i_ref;
	}
	else
	{
	    *window_push(*negative) = i_ref;
	}
    }	
    assert (range_count (positive->region) + range_count(intersect->region) + range_count(negative->region) == range_count(*input));
}

inline static phys_mesh_node * alloc_node (range_phys_mesh_node * nodes)
{
    assert (nodes->begin);
    assert(!range_is_empty(*nodes));

    return nodes->begin++;
}

static bool run_task (phys_mesh_task_pool * pool, range_phys_mesh_node * nodes)
{
    phys_mesh_task * task = phys_mesh_task_pop(pool);

    if (!task)
    {
	return false;
    }

    if (range_is_empty(task->args.region))
    {
	*task->retval = NULL;
	phys_mesh_task_free (pool, task);
	return true;
    }

    const fvec3_triangle * plane = pop_splitting_triangle(&task->args.region);

    assert (plane);

    phys_mesh_node * set = alloc_node (nodes);

    set->tri = *plane;

    if (range_is_empty(task->args.region))
    {
	log_debug("Leaf");
	set->plane.intersect =
	    set->plane.negative =
	    set->plane.positive = NULL;
    }
    else
    {
	phys_mesh_task * positive = phys_mesh_task_alloc(pool);
	phys_mesh_task * negative = phys_mesh_task_alloc(pool);
	phys_mesh_task * intersect = phys_mesh_task_alloc(pool);

	split_list (&positive->args, &intersect->args, &negative->args, plane, &task->args.region);
        
	log_debug("split %zu %zu %zu", range_count (positive->args.region) , range_count(intersect->args.region) , range_count(negative->args.region));

#define add_job(name)					\
	name->retval = &set->plane.name;		\
	phys_mesh_task_activate(pool, name);		\

	add_job(positive);
	add_job(negative);
	add_job(intersect);
    }
    
    *task->retval = set;
    phys_mesh_task_free (pool, task);
    return true;
}

static void add_initial_job (phys_mesh_task_pool * pool, range_fvec3_triangle * input)
{
    phys_mesh_task * new = phys_mesh_task_alloc (pool);

    fvec3_triangle * i;

    for_range (i, *input)
    {
	*window_push (new->args) = i;
    }

    static phys_mesh_node * dummy_retval;
    
    new->retval = &dummy_retval;

    phys_mesh_task_activate(pool, new);

    assert (range_count(new->args.region) == range_count(*input));
}

static void phys_mesh_task_pool_clear (phys_mesh_task_pool * target)
{
    phys_mesh_task * task;
    while ((task = target->free))
    {
	window_clear(task->args);
	target->free = task->pool_next;
	free (task);
    }
    
    while ((task = target->active))
    {
	window_clear(task->args);
	target->active = task->pool_next;
	free(task);
    }
}

bool phys_mesh_load (range_phys_mesh_node * result, const glb * glb)
{
    window_fvec3_triangle tris = {0};

    phys_mesh_task_pool pool = {0};

    range_phys_mesh_node to_set = {0};
    
    if (!fvec3_triangle_load_glb(&tris, glb))
    {
	log_fatal ("Could not read mesh data from glb");
    }

    add_initial_job(&pool, &tris.region);

    range_calloc(to_set, range_count(tris.region));

    *result = to_set;

    assert (range_count(pool.active->args.region) == range_count(tris.region));
    assert (range_count(tris.region) == range_count(to_set));

    while (run_task(&pool, &to_set))
    {
	log_normal("Task completed");
    }

    if (pool.active)
    {
	log_fatal ("An error occurred while building the phys_mesh");
    }

    window_clear(tris);
    phys_mesh_task_pool_clear(&pool);

    assert (range_is_empty(to_set));

    return true;

fail:
    range_clear(to_set);
    window_clear (tris);
    phys_mesh_task_pool_clear (&pool);

    return false;
}

static phys_mesh_node * phys_mesh_sweep_pop (phys_mesh_sweep * sweep)
{
    assert (!range_is_empty(sweep->todo.region));
    sweep->todo.region.end--;
    assert (*sweep->todo.region.end);
    return *sweep->todo.region.end;
}

inline static void phys_mesh_sweep_push (phys_mesh_sweep * sweep, phys_mesh_node * node)
{
    if (node)
    {
	*window_push (sweep->todo) = node;
    }
}

static fvec get_plane_distance (const fvec3_line * path, const fvec3_triangle * plane)
{
    /* 
       We should want 0 = (plane normal) dot [ [(line normal) scale (distance) + (line point)] - (plane point) ]
       Hence (distance) = [-(plane normal) dot [(line point) - (plane point)]] / [(plane normal) dot (line normal)]
    */
    
    fvec3 point_difference = { vec3_subtract_init(path->ray.vertex, plane->point) };
    return -vec3_dot(plane->normal, point_difference) / vec3_dot(plane->normal, path->ray.direction);
}

/*static bool is_point_positive (phys_mesh_node * world, fvec3 * point)
{
    fvec3 difference = vec3_subtract_init(*point, world->tri.edge[0].vertex);
    return vec3_dot(difference, world->tri.normal) >= 0;
    }*/

inline static void push_both_sides (phys_mesh_sweep * sweep, const phys_mesh_node * node)
{
    phys_mesh_sweep_push (sweep, node->plane.positive);
    phys_mesh_sweep_push (sweep, node->plane.negative);
}

inline static void push_same_side (phys_mesh_sweep * sweep, const phys_mesh_node * node)
{
    //push_both_sides (sweep, node);
    phys_mesh_sweep_push (sweep, node->plane.is_positive_index[is_point_positive (&node->tri, &sweep->result.path.ray.vertex)]);
}

/*void phys_sweep_point_plane (phys_sweep * sweep, const fvec3_plane * world)
{
    phys_sweep_update(sweep, &world->normal, get_plane_distance(&sweep->path, world));
    }*/

void phys_mesh_sweep_point_brute (phys_sweep * sweep, const range_const_phys_mesh_node * nodes)
{
    const phys_mesh_node * i;

    fvec distance;
    
    for_range (i, *nodes)
    {
	//log_debug("Normal " VEC3_FORMSPEC, VEC3_FORMSPEC_ARG(i->tri.normal));
	if (fvec3_triangle_ray_intersect_distance(&distance, &i->tri, &sweep->path.ray))
	{
	    phys_sweep_update(sweep, &i->tri.normal, distance);
	}
    }
}

void phys_mesh_sweep_point (phys_mesh_sweep * sweep)
{
    phys_mesh_node * node;
    fvec distance;

    size_t count = 0;
    
    while ( !range_is_empty (sweep->todo.region) )
    {
	count++;
	//log_debug("node");
	node = phys_mesh_sweep_pop (sweep);
	phys_mesh_sweep_push (sweep, node->plane.intersect);

	if (fvec3_triangle_ray_intersect_distance(&distance, &node->tri, &sweep->result.path.ray))
	{
	    phys_sweep_update(&sweep->result, &node->tri.normal, distance);
	    push_same_side (sweep, node);
	}
	else
	{
	    distance = get_plane_distance(&sweep->result.path, &node->tri);

	    if (distance < 0 || distance > sweep->result.path.distance)
	    {
		push_same_side (sweep, node);
	    }
	    else
	    {
		push_both_sides(sweep, node);
	    }
	}
    }

    log_debug("Percent use: %f", (float) 100.0 * (float) count / (float) sweep->_total_size);
}

void cylinder_normal (fvec3 * result, const fvec3_ray * cylinder, const fvec3 * endpoint)
{
    fvec3 delta = { vec3_subtract_init(*endpoint, cylinder->vertex) };
    fvec3_normalize(&delta, &delta);
    fvec3 cross = { vec3_cross_init(delta, cylinder->direction) };
    *result = (fvec3){ vec3_cross_init(cross, cylinder->direction) };
}

#define square(x) (x) * (x)

bool sweep_point_cylinder (phys_sweep * sweep, const fvec3 * cylinder_begin, const fvec3 * cylinder_end, fvec cylinder_radius)
{
    fvec3_line cylinder_line;

    fvec3_line_from_points(&cylinder_line, cylinder_begin, cylinder_end);

    /*fvec test = vec3_dot(cylinder_line.ray.direction, sweep->path.ray.direction);

    if (test < -FVEC_EPSILON || test > FVEC_EPSILON)
    {
	return false;
	}*/
    
    fvec3 point_delta = { vec3_subtract_init(sweep->path.ray.vertex, *cylinder_begin) };

    fvec quad_a = vec3_dot(sweep->path.ray.direction, sweep->path.ray.direction) - square(vec3_dot(sweep->path.ray.direction, cylinder_line.ray.direction));
    fvec quad_b = 2 * vec3_dot(sweep->path.ray.direction, point_delta) - 2 * vec3_dot(sweep->path.ray.direction, cylinder_line.ray.direction) * vec3_dot(point_delta, cylinder_line.ray.direction);
    fvec quad_c = vec3_dot(point_delta, point_delta) - square(vec3_dot(point_delta, cylinder_line.ray.direction)) - cylinder_radius * cylinder_radius;

    fvec quad_discriminate = quad_b * quad_b - 4 * quad_a * quad_c;

    if (quad_discriminate < 0)
    {
	return false;
    }

    fvec sqrt_quad_discriminate = sqrt(quad_discriminate);

    fvec inv_2a = 1.0 / (2.0 * quad_a);

    fvec x1 = (-quad_b + sqrt_quad_discriminate) * inv_2a;

    if (x1 < 0)
    {
	return false;
    }
    
    fvec x2 = (-quad_b - sqrt_quad_discriminate) * inv_2a;

    if (x2 < 0)
    {
	return false;
    }

    fvec distance = x1 < x2 ? x1 : x2;

    fvec3 endpoint = fvec3_line_point(&sweep->path, distance);

    fvec3 endpoint_delta = { vec3_subtract_init(endpoint, *cylinder_begin) };

    fvec hit_height = vec3_dot(endpoint_delta, cylinder_line.ray.direction);

    //log_debug("math error %f", error);

    if (hit_height < 0 || hit_height > cylinder_line.distance)
    {
	return false;
    }

    fvec error = vec3_dot(endpoint_delta, endpoint_delta) - cylinder_radius * cylinder_radius - hit_height * hit_height;
    //fvec error2 = quad_a * distance * distance + quad_b * distance + quad_c;

    if (error > FVEC_EPSILON)
    {
	//log_debug("math error %f,%f at hit height %f/%f", error, error2, hit_height, cylinder_line.distance);
	//return false;
    }
    else
    {
	//log_debug("No error");
    
	//log_debug("Directions " VEC3_FORMSPEC " dot " VEC3_FORMSPEC " = %f", VEC3_FORMSPEC_ARG(sweep->path.ray.direction), VEC3_FORMSPEC_ARG(cylinder_line.ray.direction), vec3_dot(sweep->path.ray.direction, cylinder_line.ray.direction));
    }
    
    return phys_sweep_update_no_normal(sweep, distance);
    
    /*fvec3_line cylinder_line;

    fvec3_line_from_points(&cylinder_line, cylinder_begin, cylinder_end);

    fvec3_ray sweep_transform;

    fvec3_ray_transform(&sweep_transform, &cylinder_line.ray, &sweep->path.ray);

    //log_debug("normal " VEC3_FORMSPEC " from " VEC3_FORMSPEC, VEC3_FORMSPEC_ARG(sweep_transform.direction), VEC3_FORMSPEC_ARG(sweep->path.ray.direction));

    fvec quadratic_a = vec2_dot(sweep_transform.direction, sweep_transform.direction);

    if (quadratic_a < FVEC_EPSILON)
    {
	return false;
    }
    
    fvec quadratic_b = 2 * vec2_dot(sweep_transform.direction, sweep_transform.vertex);
    fvec quadratic_c = vec2_dot(sweep_transform.vertex, sweep_transform.vertex) - cylinder_radius * cylinder_radius;

    fvec discriminate = quadratic_b * quadratic_b - 4 * quadratic_a * quadratic_c;

    if (discriminate < 0)
    {
	return false;
    }

    fvec sqrt_discriminate = sqrt(discriminate);

    fvec inv_2a = 1.0 / (2 * quadratic_a);

    fvec x1 = (-quadratic_b + sqrt_discriminate) * inv_2a;

    if (x1 < 0)
    {
	return false;
    }
    
    fvec x2 = (-quadratic_b - sqrt_discriminate) * inv_2a;

    if (x2 < 0)
    {
	return false;
    }

    fvec distance = x1 < x2 ? x1 : x2;

    fvec height = distance * sweep_transform.direction.z;

    if (height < 0 || height > cylinder_line.distance)
    {
	return false;
    }

    if (!phys_sweep_update_no_normal (sweep, distance))
    {
	return false;
    }

    fvec3 endpoint = fvec3_line_point(&sweep->path, sweep->path.distance);

    cylinder_normal(&sweep->hit_normal, &cylinder_line.ray, &endpoint);

    return true;*/
}
bool sweep_point_sphere (phys_sweep * sweep, const fvec3 * sphere_position, fvec sphere_radius)
{
    fvec3 position_transform = { vec3_subtract_init(sweep->path.ray.vertex, *sphere_position) };

    fvec quad_a = vec3_dot(sweep->path.ray.direction, sweep->path.ray.direction);
    fvec quad_b = 2 * vec3_dot(sweep->path.ray.direction, position_transform);
    fvec quad_c = vec3_dot(position_transform, position_transform) - sphere_radius * sphere_radius;

    fvec quad_discriminate = quad_b * quad_b - 4 * quad_a * quad_c;

    if (quad_discriminate < 0)
    {
	return false;
    }

    fvec sqrt_quad_discriminate = sqrt(quad_discriminate);

    fvec inv_2a = 1.0 / (2.0 * quad_a);

    fvec x1 = (-quad_b + sqrt_quad_discriminate) * inv_2a;

    if (x1 < 0)
    {
	return false;
    }
    
    fvec x2 = (-quad_b - sqrt_quad_discriminate) * inv_2a;

    if (x2 < 0)
    {
	return false;
    }

    fvec distance = x1 < x2 ? x1 : x2;

    if (!phys_sweep_update_no_normal(sweep, distance))
    {
	return false;
    }

    fvec3 endpoint = fvec3_line_point(&sweep->path, distance);

    fvec3 endpoint_delta = { vec3_subtract_init(endpoint, *sphere_position) };

    fvec3_normalize(&sweep->hit_normal, &endpoint_delta);
    
    return true;
}

bool sweep_point_triangle (phys_sweep * sweep, fvec3_triangle * world)
{
    fvec distance;
    if (fvec3_triangle_ray_intersect_distance(&distance, world, &sweep->path.ray))
    {
	return phys_sweep_update(sweep, &world->normal, distance);
    }
    else
    {
	return false;
    }
}

void phys_mesh_sweep_aligned_ellipsoid (phys_mesh_sweep * sweep, fvec3_aligned_ellipsoid * shape)
{
    phys_mesh_node * node;
    //fvec distance;

    size_t count = 0;

    struct {
	fvec3_triangle unit_sphere;
	fvec3_triangle point;
    }
	transform;

    fvec begin_distance;
    fvec end_distance;

    while ( !range_is_empty (sweep->todo.region) )
    {
	count++;
	//log_debug("node");
	node = phys_mesh_sweep_pop (sweep);
	phys_mesh_sweep_push (sweep, node->plane.intersect);

	fvec3_aligned_ellipsoid_unit_sphere_transform_triangle(&transform.unit_sphere, shape, &node->tri);
	fvec3_unit_sphere_point_transform_triangle(&transform.point, &transform.unit_sphere);

#if 1
	if (sweep_point_triangle(&sweep->result, &transform.point))
	{
	    push_same_side(sweep, node);
	    continue;
	}

	begin_distance = fvec3_plane_distance(transform.unit_sphere, sweep->result.path.ray.vertex);
	end_distance = fvec3_plane_distance(transform.unit_sphere, sweep->result.endpoint);

	if ( (begin_distance < -1 && end_distance < -1) || (begin_distance > 1 && end_distance > 1) )
	{
	    push_same_side(sweep, node);
	    continue;
	}
	
	push_both_sides (sweep, node);
	
	/*push_both_sides (sweep, node);
	if (fvec3_triangle_ray_intersect_distance(&distance, &transform.point, &sweep->result.path))
	{
	    phys_sweep_update(&sweep->result, &node->tri.normal, distance);
	    continue;
	    }*/
#else
	if (fvec3_triangle_ray_intersect_distance(&distance, &transform.point, &sweep->result.path))
	{
	    phys_sweep_update(&sweep->result, &node->tri.normal, distance);
	    push_same_side (sweep, node);
	    continue;
	}
	else
	{
	    distance = get_plane_distance(&sweep->result.path, &transform.point);

	    if (distance < 0 || distance > sweep->result.path.distance)
	    {
		push_same_side (sweep, node);
		continue;
	    }
	    else
	    {
		push_both_sides(sweep, node);
	    }
	}
#endif
	sweep_point_cylinder(&sweep->result, node->tri.vertex, node->tri.vertex + 1, 1);
	sweep_point_cylinder(&sweep->result, node->tri.vertex + 1, node->tri.vertex + 2, 1);
	sweep_point_cylinder(&sweep->result, node->tri.vertex + 2, node->tri.vertex, 1);

	sweep_point_sphere(&sweep->result, node->tri.vertex, 1);
	sweep_point_sphere(&sweep->result, node->tri.vertex + 1, 1);
	sweep_point_sphere(&sweep->result, node->tri.vertex + 2, 1);

	
    }

    //log_debug("Percent use: %f", (float) 100.0 * (float) count / (float) sweep->_total_size);
}
