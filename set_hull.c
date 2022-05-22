#include <assert.h>
#include "set_hull.h"
#include "../log/log.h"
#include "mesh/load.h"
#include "mesh/build.h"
#include "../window/alloc.h"
#include "../range/alloc.h"

bool phys_object_set_hull (phys_object * object, const glb * mesh)
{
    window_phys_mesh_tri tris = {0};
    
    if (!phys_mesh_tri_load_glb(&tris, mesh))
    {
	log_fatal ("Could not load tris from glb");
    }

    range_clear (object->mesh);
    
    phys_mesh_build (&object->mesh, &tris.region.alias_const);
    
    window_clear (tris);

    return true;

fail:
    window_clear(tris);
    return false;
}
