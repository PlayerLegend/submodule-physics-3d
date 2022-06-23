#include "load.h"
#include "../../../window/alloc.h"
#include "../../../gltf/env.h"
#include "../../../log/log.h"
#include <stdlib.h>
#include "../../../range/alloc.h"

static void fvec3_triangle_from_fvec3 (fvec3_triangle * result, fvec3 positions[3])
{
    /*fvec3 a = { vec3_subtract_init(positions[0], positions[1]) };
    fvec3 b = { vec3_subtract_init(positions[0], positions[2]) };

    fvec3 cross = { vec3_cross_init (a, b) };

    fvec cross_scale = 1.0 / fvec3_vlen(&cross);

    vec3_scale (cross, cross_scale);

    result->normal = cross;*/

    result->vertex[0] = positions[0];
    result->vertex[1] = positions[1];
    result->vertex[2] = positions[2];

    fvec3_triangle_set_normal(result);
    
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
	//log_debug("Leaf");
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
        
	//log_debug("split %zu %zu %zu", range_count (positive->args.region) , range_count(intersect->args.region) , range_count(negative->args.region));

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

bool phys_mesh_node_load (range_phys_mesh_node * result, const glb * glb)
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
	//log_normal("Task completed");
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
