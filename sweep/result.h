#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include "../../vec/shape3.h"
#endif

typedef struct {
    bool is_hit;
    fvec distance;
    fvec3 hit_normal;
    fvec3 end;
}
    phys_sweep_result;
