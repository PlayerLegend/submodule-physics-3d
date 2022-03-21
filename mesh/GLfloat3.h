#ifndef FLAT_INCLUDES
#include <stddef.h>
#include "../../../glad/include/glad/glad.h"
#define FLAT_INCLUDES
#include "../../vec/vec.h"
#include "../../vec/vec3.h"
#include "../../range/def.h"
#include "../../window/def.h"
#include "../../gltf/accessor_loaders/GLfloat3.h"
#include "def.h"
#endif

void phys_mesh_tri_from_GLfloat3 (phys_mesh_tri * result, GLfloat3 positions[3]);
