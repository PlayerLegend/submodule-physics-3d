#include <assert.h>
#include "set_hull.h"
#include "../log/log.h"
#include "mesh.h"
#include "../window/alloc.h"
#include "../range/alloc.h"

bool phys_object_set_hull (phys_object * object, const glb * mesh)
{
    if (!phys_mesh_load(&object->mesh, mesh))
    {
	log_fatal ("Could not load tris from glb");
    }

    return true;

fail:
    return false;
}
