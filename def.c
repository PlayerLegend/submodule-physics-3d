#include "def.h"
#include "../range/alloc.h"
#include "../window/alloc.h"
#include <stdlib.h>

void phys_object_clear (phys_object * target)
{
    range_clear(target->mesh);
    *target = (phys_object) {0};
}

void phys_mesh_sweep_start (phys_mesh_sweep * target, fvec3_line * path, const range_phys_mesh_node * mesh)
{
    window_rewrite(target->todo);
    *window_push(target->todo) = mesh->begin;
    phys_sweep_start(&target->result, path);
    target->_total_size = range_count(*mesh);
}
