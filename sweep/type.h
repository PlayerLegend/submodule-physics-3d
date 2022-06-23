#ifndef FLAT_INCLUDES
#include "../../vec/shape3.h"
#endif

typedef struct {
    fvec3_combined_line path;
    fvec3 hit_position;
    fvec3 hit_normal;
    bool hit;
}
    phys_sweep_result;
