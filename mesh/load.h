#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../../vec/range_vec3.h"
#include "../../convert/source.h"
#include "def.h"
#endif

bool phys_mesh_tri_load (window_phys_mesh_tri * result, convert_source * source);
bool phys_mesh_node_load (range_phys_mesh_node * nodes, convert_source * source);
