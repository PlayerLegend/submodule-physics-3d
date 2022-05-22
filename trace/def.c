#include "def.h"
#include "../../vec/trace3.h"

void phys_trace_transform (vec_trace3 * result, const vec_object3 * object, const vec_trace3 * trace)
{
    result->position = (fvec3) vec3_subtract_init(trace->position, object->position);
    vec4_quaternion_unrotate(&result->position, &object->quaternion, &result->position);
    vec4_quaternion_unrotate(&result->direction, &object->quaternion, &trace->direction);
    result->distance = trace->distance / object->scale;
}

void phys_trace_untransform (phys_trace_result * result, const vec_object3 * object, const phys_trace_result * input)
{
    result->distance = input->distance * object->scale;
    vec4_quaternion_rotate(&result->hit_normal, &object->quaternion, &input->hit_normal);
    vec4_quaternion_rotate(&result->end, &object->quaternion, &input->end);
    vec3_add(result->end, object->position);
}
