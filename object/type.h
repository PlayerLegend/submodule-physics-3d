#ifndef FLAT_INCLUDES
#include "../mesh/type.h"
#endif

typedef struct {
    vec_object3 * origin;
}
    phys_object_point;

typedef struct {
    vec_object3 * origin;
    fvec3 radius;
}
    phys_object_ellipsoid;

typedef struct {
    vec_object3 * origin;
    phys_mesh hull;
}
    phys_object_mesh;

typedef enum {
    PHYS_OBJECT_POINT,
    PHYS_OBJECT_ELLIPSOID,
    PHYS_OBJECT_MESH,
}
    phys_object_type;

typedef struct {
    phys_object_type type;
    union {
	vec_object3 * origin;
	phys_object_point point;
	phys_object_ellipsoid ellipsoid;
	phys_object_mesh mesh;
    };
}
    phys_object;
