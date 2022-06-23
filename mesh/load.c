#include "load.h"
#include "node/load.h"
#include "boundary/set.h"

bool phys_mesh_load (phys_mesh * result, const glb * glb)
{
    if (!phys_mesh_node_load(&result->nodes, glb))
    {
	return false;
    }

    phys_mesh_boundary_set(&result->boundary, &result->nodes);

    return true;
}
