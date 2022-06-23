#include "free.h"
#include "../../range/alloc.h"
#include <stdlib.h>

void phys_mesh_clear(phys_mesh * target)
{
    range_clear(target->nodes);
}

void phys_mesh_free(phys_mesh * target)
{
    phys_mesh_clear(target);
    free(target);
}
