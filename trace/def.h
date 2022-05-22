#ifndef FLAT_INCLUDES
#include "../def.h"
#endif

/*typedef struct {
    vec_trace3 trace;
    const vec_object3 * object;
}
phys_trace_arg;*/

typedef struct {
    bool is_hit;
    fvec distance;
    fvec3 hit_normal;
    fvec3 end;
}
    phys_trace_result;

#define phys_trace_result_init(trace)		\
    (phys_trace_result)	{						\
	.distance = (trace).distance,					\
	    .end = { (trace).position.x + (trace).distance * (trace).direction.x, \
	    (trace).position.y + (trace).distance * (trace).direction.y, \
	    (trace).position.z + (trace).distance * (trace).direction.z,} \
      }
