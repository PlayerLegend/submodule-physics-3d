#ifndef FLAT_INCLUDES
#include "def.h"
#include "../../vec/trace3.h"
#endif

void phys_trace_line_tri (phys_trace_result * result, const phys_mesh_tri * tri, const vec_trace3 * arg);
void phys_trace_line_mesh_subset (phys_trace_result * result, const range_const_phys_mesh_tri_p * set, const vec_trace3 * trace);
void phys_trace_line_object (phys_trace_result * result, phys_object * object, const vec_trace3 * trace);
