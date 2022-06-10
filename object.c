#include "object.h"
#include "../log/log.h"

void phys_object_sweep_point (phys_object * object, phys_mesh_sweep * sweep, fvec3_line * path)
{
    object3_transform_line (&sweep->result.path, object->origin, path);
    phys_mesh_sweep_start(sweep, &sweep->result.path, &object->mesh);
    phys_mesh_sweep_point(sweep);
    object3_untransform_line(&sweep->result.path, object->origin, &sweep->result.path);
    object3_untransform_direction(&sweep->result.hit_normal, object->origin, &sweep->result.hit_normal);
}

void phys_object_sweep_aligned_ellipsoid (phys_object * object, phys_mesh_sweep * sweep, fvec3_line * path, fvec3_aligned_ellipsoid * shape)
{
    object3_transform_line (&sweep->result.path, object->origin, path);
    phys_mesh_sweep_start(sweep, &sweep->result.path, &object->mesh);
    phys_mesh_sweep_aligned_ellipsoid (sweep, shape);
    object3_untransform_line(&sweep->result.path, object->origin, &sweep->result.path);
    object3_untransform_direction(&sweep->result.hit_normal, object->origin, &sweep->result.hit_normal);
}
