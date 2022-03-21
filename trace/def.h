#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#endif

typedef struct {
    fvec3 origin;
    fvec3 direction;
    fvec distance;
}
    phys_trace_arg;

typedef struct {
    bool is_hit;
    fvec distance;
    fvec3 hit_normal;
    fvec3 end;
}
    phys_trace_result;
