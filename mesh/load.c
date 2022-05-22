#include "load.h"
#include "../../convert/status.h"
#include "../../convert/source.h"
#include "../../convert/fd/source.h"
#include "../../gltf/convert.h"
#include "../../gltf/env.h"
#include "../../log/log.h"
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include "build.h"
#include "../../window/alloc.h"
#include "../../gltf/parse.h"

static void phys_mesh_tri_from_fvec3 (phys_mesh_tri * result, fvec3 positions_orig[3])
{
    fvec3 positions_hack[3] =
    {
	positions_orig[0],
	positions_orig[1],
	positions_orig[2],
    };
    
    fvec3 a = vec3_subtract_init(positions_hack[0], positions_hack[1]);
    fvec3 b = vec3_subtract_init(positions_hack[0], positions_hack[2]);

    fvec3 cross = vec3_cross (a, b);

    fvec cross_scale = 1.0 / sqrt (cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);

    vec3_scale (cross, cross_scale);

    result->position[0] = (fvec3) vec3_initializer (positions_hack[0]);
    result->position[1] = (fvec3) vec3_initializer (positions_hack[1]);
    result->position[2] = (fvec3) vec3_initializer (positions_hack[2]);
    
    result->normal = cross;
}

typedef struct {
    window_phys_mesh_tri * tris;
    fvec3 positions[3];
    int count;
}
    phys_mesh_tri_target;

static bool phys_mesh_tri_loader(void * target_void, const fvec3 * input)
{
    phys_mesh_tri_target * target = target_void;

    target->positions[target->count++] = *input;

    if (target->count == 3)
    {
	target->count = 0;

	phys_mesh_tri_from_fvec3(window_push(*target->tris), target->positions);
    }

    return true;
}

bool phys_mesh_tri_load_glb (window_phys_mesh_tri * result, const glb * glb)
{
    gltf_accessor_env env;
    gltf_mesh * mesh;
    gltf_mesh_primitive * primitive;
    window_gltf_index indices = {0};
    phys_mesh_tri_target target = { .tris = result };
    
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

bool phys_mesh_tri_load_source (window_phys_mesh_tri * result, convert_source * source)
{
    glb glb = {0};
    
    if (!glb_load_from_source(&glb, source))
    {
	log_fatal ("Could not load from source");
    }

    if (!phys_mesh_tri_load_glb (result, &glb))
    {
	log_fatal ("Could not load tris from glb");
    }

    glb_clear (&glb);

    return true;
    
fail:
    glb_clear (&glb);
    return false;
}



/*
bool phys_mesh_tri_load_path (window_phys_mesh_tri * result, const char * path)
{
    window_unsigned_char buffer = {0};

    fd_source fd_source = fd_source_init (open (path, O_RDONLY), &buffer);

    if (fd_source.fd < 0)
    {
	perror (path);
	log_fatal ("Could not open file for triangle loading");
    }

    if (!phys_mesh_tri_load_source(result, &fd_source.source))
    {
	log_fatal ("Failed to load triangles from %s", path);
    }

    convert_source_clear (&fd_source.source);
    
    return true;

fail:
    convert_source_clear (&fd_source.source);
    return false;
}
*/
/*
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

bool phys_mesh_node_load_path (range_phys_mesh_node * nodes, const char * path)
{
    window_unsigned_char buffer = {0};

    fd_source fd_source = fd_source_init (open (path, O_RDONLY), &buffer);

    if (fd_source.fd < 0)
    {
	perror (path);
	log_fatal ("Could not open file for physics mesh generation");
    }

    if (!phys_mesh_node_load(nodes, &fd_source.source))
    {
	log_fatal ("Failed to load from source");
    }
    
    convert_source_clear (&fd_source.source);

    return true;

fail:
    convert_source_clear (&fd_source.source);
    return false;
}
*/
