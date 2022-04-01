#ifndef FLAT_INCLUDES
#include "def.h"
#include "../../convert/status.h"
#include "../../convert/source.h"
#include <stdbool.h>
#include <stdint.h>
#include "../../gltf/def.h"
#endif

bool phys_mesh_tri_load_glb (window_phys_mesh_tri * result, const glb * glb);
bool phys_mesh_tri_load_source (window_phys_mesh_tri * result, convert_source * source);
//bool phys_mesh_node_load (range_phys_mesh_node * nodes, convert_source * source);
//bool phys_mesh_node_load_path (range_phys_mesh_node * nodes, const char * path);
