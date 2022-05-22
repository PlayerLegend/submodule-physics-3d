#include "def.h"
#include "../range/alloc.h"
#include "../window/alloc.h"

void phys_object_clear (phys_object * target)
{
    range_clear (target->mesh);
    window_clear (target->subset);
    *target = (phys_object) {0};
}
