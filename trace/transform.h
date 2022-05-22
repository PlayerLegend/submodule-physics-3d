#ifndef FLAT_INCLUDES
#include "def.h"
#include "../../vec/trace3.h"
#endif

void phys_trace_transform (vec_trace3 * result, const vec_object3 * object, const vec_trace3 * trace);
void phys_trace_untransform (phys_trace_result * result, const vec_object3 * object, const phys_trace_result * input);

