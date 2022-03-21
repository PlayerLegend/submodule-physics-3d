#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../glad/include/glad/glad.h"
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../../window/alloc.h"
#include "../../vec/range_vec3.h"
#include "../../convert/source.h"
#include "def.h"
#include "../../keyargs/keyargs.h"
#include "../../gltf/def.h"
#include "../../gltf/env.h"
#include "../../gltf/convert.h"
#include "../../log/log.h"
#include "build.h"
#include "load.h"


void phys_mesh_tri_from_fvec3 (phys_mesh_tri * result, fvec3 positions_orig[3])
{
    fvec3 positions_hack[3] =
/*	{
	    vec3_scale_init(positions_orig[0], -1),
	    vec3_scale_init(positions_orig[1], -1),
	    vec3_scale_init(positions_orig[2], -1)
	    };*/
    {
	positions_orig[0],
	positions_orig[1],
	positions_orig[2],
    };
    
    fvec3 a = vec3_subtract_init(positions_hack[0], positions_hack[1]);
    fvec3 b = vec3_subtract_init(positions_hack[0], positions_hack[2]);

    //log_debug ("delta [%f %f %f] [%f %f %f]", a.x, a.y, a.z, b.x, b.y, b.z);
    
    fvec3 cross = vec3_cross (a, b);

    fvec cross_scale = 1.0 / sqrt (cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);

    vec3_scale (cross, cross_scale);

    result->position[0] = (fvec3) vec3_initializer (positions_hack[0]);
    result->position[1] = (fvec3) vec3_initializer (positions_hack[1]);
    result->position[2] = (fvec3) vec3_initializer (positions_hack[2]);
    
    result->normal = cross;

    //log_debug ("Loaded [%f %f %f]", cross.x, cross.y, cross.z);
}

typedef struct {
    window_phys_mesh_tri * tris;
    fvec3 positions[3];
    int count;
}
    phys_mesh_tri_target;

bool phys_mesh_tri_loader(void * target_void, const fvec3 * input)
{
    phys_mesh_tri_target * target = target_void;

    target->positions[target->count++] = *input;

    if (target->count == 3)
    {
	target->count = 0;

	/*log_debug ("positions [%f %f %f] [%f %f %f] [%f %f %f]",
		   target->positions[0].x,
		   target->positions[0].y,
		   target->positions[0].z,
		   target->positions[1].x,
		   target->positions[1].y,
		   target->positions[1].z,
		   target->positions[2].x,
		   target->positions[2].y,
		   target->positions[2].z);*/
	
	phys_mesh_tri_from_fvec3(window_push(*target->tris), target->positions);
    }

    return true;
}

bool phys_mesh_tri_load (window_phys_mesh_tri * result, convert_source * source)
{
    glb_toc toc;
    gltf gltf;
    gltf_accessor_env env;
    gltf_mesh * mesh;
    gltf_mesh_primitive * primitive;
    window_gltf_index indices = {0};
    phys_mesh_tri_target target = { .tris = result };

    if (!gltf_load_from_source(&gltf, &toc, source))
    {
	log_fatal ("Could not load from source");
    }

    for_range(mesh, gltf.meshes)
    {
	for_range(primitive, mesh->primitives)
	{
	    if (!gltf_accessor_env_setup (&env, &toc, primitive->indices))
	    {
		log_fatal ("Invalid index accessor");
	    }

	    window_rewrite(indices);

	    if (!gltf_accessor_env_load_indices (&indices, &env))
	    {
		log_fatal ("Could not load indicies");
	    }

	    if (!gltf_accessor_env_setup (&env, &toc, primitive->attributes.position))
	    {
		log_fatal ("Invalid position accessor");
	    }
	    
	    if (!gltf_accessor_env_load_fvec3(&target, phys_mesh_tri_loader, &indices.region, &env))
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

bool phys_mesh_node_load (range_phys_mesh_node * nodes, convert_source * source)
{
    window_phys_mesh_tri tris = {0};
    
    if (!phys_mesh_tri_load(&tris, source))
    {
	free (tris.alloc.begin);
	return false;
    }

    phys_mesh_build (nodes, &tris.region.const_cast);
    
    free (tris.alloc.begin);
    return true;
}
