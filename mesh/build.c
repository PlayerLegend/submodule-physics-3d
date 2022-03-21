#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../vec/range_vec3.h"
#include "../../window/def.h"
#include "../../window/alloc.h"
#include "def.h"
#include "build.h"
#include "../../log/log.h"

range_typedef(size_t,size_t);
window_typedef(size_t, size_t);
window_typedef(phys_mesh_node,phys_mesh_node);
window_typedef(fvec3,fvec3);

typedef struct task task;

struct task {
    window_size_t list;
    size_t * result;

    task * next;
};

static task * task_pop (task ** pool)
{
    task * retval = *pool;
    //log_debug ("pop %p", retval);

    assert (retval);
    
    *pool = retval->next;
    retval->next = NULL;

    return retval;
}

static task * task_alloc (task ** pool, size_t * node_index)
{
    task * retval = *pool;

    if (!retval)
    {
	retval = calloc (1, sizeof(*retval));
    }
    else
    {
	*pool = retval->next;
	retval->next = NULL;
	window_rewrite (retval->list);
    }

    retval->result = node_index;

    return retval;
}

static void task_push (task ** pool, task * target)
{
    //log_debug ("push %p", target);
    
    target->next = NULL;

    target->next = *pool;
    *pool = target;
}

static void pool_free (task ** pool)
{
    task * to_free;
    
    while (*pool)
    {
	to_free = *pool;
	*pool = to_free->next;
	free (to_free->list.alloc.begin);
	free (to_free);
    }
}

typedef struct {
    task * positive;
    task * negative;
    task * cross;
}
    sorted_lists;

static void find_list_center (fvec3 * center, const range_size_t * list, const range_const_phys_mesh_tri * tris)
{
    const phys_mesh_tri * tri;
    const size_t * index;

    *center = (fvec3){0};
    
    for_range (index, *list)
    {
	tri = tris->begin + *index;

	vec3_add(*center, tri->position[0]);
	vec3_add(*center, tri->position[1]);
	vec3_add(*center, tri->position[2]);
    }

    double scale = 1.0 / (double) range_count (*list);

    vec3_scale(*center, scale);

    //log_debug ("Center [%f %f %f]", center->x, center->y, center->z);
}

static double calc_dist (const phys_mesh_tri * tri, const fvec3 * point)
{
    fvec3 displacement = vec3_subtract_init(*point, tri->position[0]);

    return vec3_dot(displacement, tri->normal);
}

static size_t * find_splitting_tri (const range_size_t * list, const range_const_phys_mesh_tri * tris)
{
    fvec3 list_center;

    find_list_center (&list_center, list, tris);

    size_t * best_index = list->begin;
    double dist;
    double best_dist = calc_dist (tris->begin + *best_index, &list_center);
    if (best_dist < 0)
    {
	best_dist = -best_dist;
    }
    
    const phys_mesh_tri * tri;
    size_t * index;

    range_size_t list_rest = { .begin = list->begin + 1, .end = list->end };

    for_range (index, list_rest)
    {
	tri = tris->begin + *index;

	dist = calc_dist (tri, &list_center);

	if (dist < 0)
	{
	    dist = -dist;
	}

	//log_debug ("dist %f", dist);
	    
	if (dist < best_dist)
	{
	    //log_debug ("dist fix %f < %f", dist, best_dist);
	    best_dist = dist;
	    best_index = index;
	}
    }

    return best_index;
}

static void sort_list (sorted_lists * sorted, const phys_mesh_tri * plane, const range_const_size_t * list, const range_const_phys_mesh_tri * tris)
{
    assert (range_count (*list) >= 1);
    
    const size_t * index;
    const phys_mesh_tri * tri;

    fvec3 offset;
    
    float dot[3];

    size_t tri_index;

    assert (list != &sorted->cross->list.region.const_cast);
    assert (list != &sorted->positive->list.region.const_cast);
    assert (list != &sorted->negative->list.region.const_cast);

    //log_debug ("sorting %d", range_count(*list));
	
    for_range (index, *list)
    {
	tri = tris->begin + *index;

	assert (tri != plane);

#define calc_tri_dot(i)				\
	offset = (fvec3) vec3_subtract_init (tri->position[i], plane->position[0]); \
	dot[i] = vec3_dot (offset, plane->normal);

	calc_tri_dot(0);
	calc_tri_dot(1);
	calc_tri_dot(2);

	tri_index = range_index(tri, *tris);

	if (dot[0] >= 0 && dot[1] >= 0 && dot[2] >= 0)
	{
	    *window_push (sorted->positive->list) = tri_index;
	}
	else if (dot[0] <= 0 && dot[1] <= 0 && dot[2] <= 0)
	{
	    *window_push (sorted->negative->list) = tri_index;
	}
	else
	{
	    *window_push (sorted->cross->list) = tri_index;
	}
    }

    /*log_debug ("diff %d, count %d", range_count (*list)
	       -range_count(sorted->positive->list.region)
	       -range_count(sorted->cross->list.region)
	       -range_count(sorted->negative->list.region),
	       range_count(*list));*/
    
    assert (range_count (*list) ==
	    +range_count(sorted->positive->list.region)
	    +range_count(sorted->cross->list.region)
	    +range_count(sorted->negative->list.region));
    
    log_debug ("%3d = %3d %3d %3d",
	       range_count (*list),
	       range_count(sorted->positive->list.region),
	       range_count(sorted->cross->list.region),
	       range_count(sorted->negative->list.region));

}

typedef struct {
    task * free;
    task * used;
    size_t node_count;
}
    state;

void run_iteration (range_phys_mesh_node * nodes, state * state, task * next, const range_const_phys_mesh_tri * tris)
{
    assert (next->result);

    if (range_is_empty (next->list.region))
    {
	*next->result = 0;
	//log_debug ("empty");
	return;
    }

    //log_debug ("iteration");
    
    *next->result = state->node_count++;

    phys_mesh_node * node = nodes->begin + *next->result;

    assert (node < nodes->end);

    if (range_count (next->list.region) == 1)
    {
	assert ((*next->list.region.begin) < (size_t)range_count (*tris));
	*node = (phys_mesh_node)
	{
	    .tri = tris->begin[*next->list.region.begin],
	};
	
	return;
    }
    
    sorted_lists lists =
    {
	.positive = task_alloc(&state->free, &node->plane_positive),
	.negative = task_alloc(&state->free, &node->plane_negative),
	.cross = task_alloc(&state->free, &node->plane_cross),
    };

    size_t * plane_index_p = find_splitting_tri(&next->list.region, tris);

    size_t plane_index = *plane_index_p;

    assert (range_count (next->list.region) > 1);
    
    *plane_index_p = *(--next->list.region.end);
    
    const phys_mesh_tri * plane = tris->begin + plane_index;
    
    sort_list(&lists, plane, &next->list.region.const_cast, tris);

    task_push (&state->used, lists.positive);
    task_push (&state->used, lists.negative);
    task_push (&state->used, lists.cross);

    node->tri = *plane;
}

void phys_mesh_build (range_phys_mesh_node * nodes, const range_const_phys_mesh_tri * tris)
{
    size_t tri_count = range_count(*tris);

    range_calloc (*nodes, tri_count);

    state build_state = {0};

    size_t dummy_index;

    task * initial_task = task_alloc (&build_state.free, &dummy_index);

    //assert (dummy_index == 0);
    
    for (size_t i = 0; i < tri_count; i++)
    {
	*window_push (initial_task->list) = i;
    }

    //log_debug ("count %p: %d", initial_task, range_count(initial_task->list.region));
    
    task_push (&build_state.used, initial_task);
    
    //log_debug ("count2 %p: %d", initial_task, range_count(initial_task->list.region));
    
    task * next_task;
    
    while (build_state.used)
    {
	next_task = task_pop (&build_state.used);
	//log_debug ("count %p: %d", next_task, range_count(next_task->list.region));

	run_iteration(nodes, &build_state, next_task, tris);

	next_task->result = NULL;

	task_push (&build_state.free, next_task);
    }

    pool_free (&build_state.free);
}
