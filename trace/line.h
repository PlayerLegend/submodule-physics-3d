#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "def.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../mesh/def.h"
#endif

void phys_trace_line_tri (phys_trace_result * result, const phys_mesh_tri * tri, const phys_trace_arg * arg);
void phys_trace_line_mesh_subset (phys_trace_result * result, const range_const_phys_mesh_tri_p * set, const phys_trace_arg * arg);
