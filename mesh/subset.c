/*#include <stddef.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "def.h"*/
#include "subset.h"
#include <assert.h>
#include "../../window/alloc.h"

void phys_mesh_subset_all (window_phys_mesh_tri_p * result, const range_const_phys_mesh_node * nodes)
{
    const phys_mesh_node * node;

    window_rewrite (*result);
    
    for_range (node, *nodes)
    {
	*window_push (*result) = (phys_mesh_tri*)&node->tri;
    }
}
