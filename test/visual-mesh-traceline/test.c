#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include "../../object/set.h"
#include "../../../draw-3d/mesh/type.h"
#include "../../../draw-3d/object/type.h"
#include "../../../draw-3d/buffer/type.h"
#include "../../../draw-3d/shader/type.h"
#include "../../../draw-3d/object/set.h"
#include "../../../draw-3d/buffer/load/path.h"
#include "../../../draw-3d/shader/load/path.h"
#include "../../../draw-3d/shader/link.h"
#include "../../../draw-3d/shader/free.h"
#include "../../../draw-3d/buffer/index.h"
#include "../../../draw-3d/buffer/display.h"
#include "../../../draw-3d/buffer/free.h"
#include "../../../convert/type.h"
#include "../../../convert/source.h"
#include "../../../convert/fd/source.h"
#include "../../../keyargs/keyargs.h"
#include "../../../game-interface/window.h"
#include "../../../game-interface/init.h"
#include "../../../game-interface/time.h"
#include "../../../range/alloc.h"
#include "../../../glad/glad.h"
#include "../../../window/alloc.h"
#include "../../../log/log.h"
#include "../../sweep/type.h"
#include "../../sweep/moving-point-against-mesh.h"
#include "../../../thread/benchmark.h"
#include "../../../gltf/load/path.h"
#include "../../../gltf/free.h"

typedef struct {
    vec_object3 origin;
    draw_object draw;
    phys_object phys;
}
    entity;

inline static void entity_init(entity * target, const char * phys_mesh_path, draw_mesh * draw_mesh)
{
    *target = (entity){0};
    target->draw.origin = &target->origin;
    target->phys.origin = &target->origin;
    target->origin = VEC_OBJECT3_INITIALIZER;
    if (phys_mesh_path)
    {
	glb mesh;

	assert(glb_load_path(&mesh, phys_mesh_path));
	phys_object_set_mesh(&target->phys, &mesh);
	glb_clear(&mesh);
    }
    else
    {
	phys_object_set_point(&target->phys);
    }
    
    draw_object_set_mesh(&target->draw, draw_mesh);
}

int main(int argc, char * argv[])
{
    assert (ui_init());
    
    ui_window * window;
    assert (ui_window_new(&window, .width = 640, .height = 480, .title = "draw-3d test"));
    assert (window);
    
    draw_buffer * buffer;
    assert (draw_buffer_load_path(&buffer, 3, argv + 1));
    assert (buffer);

    draw_shader_vertex * vertex;
    assert (draw_shader_vertex_load_path(&vertex, argv[4]));

    draw_shader_fragment * fragment;
    assert (draw_shader_fragment_load_path(&fragment, argv[5]));

    draw_shader_program * shader;
    assert (draw_shader_program_link(&shader, vertex, fragment));
    draw_shader_vertex_free(vertex);
    draw_shader_fragment_free(fragment);

    entity solid;
    entity_init(&solid, argv[1], draw_buffer_index(buffer, 0));

    entity trace;
    entity_init(&trace, NULL, draw_buffer_index(buffer, 1));

    entity normal;
    entity_init(&normal, NULL, draw_buffer_index(buffer, 2));

    draw_view view = { .quaternion = { .w = 1 }, .position.z = 5 };

    phys_sweep_result sweep_result;

    size_t frame_counter = 0;
    
    while (!ui_window_should_close(window))
    {
	frame_counter++;
	fvec time = ui_get_time();
	trace.origin.position = (fvec3){ .y = -10 };
	fvec4_setup_rotation_quaternion(&solid.origin.quaternion, &(fvec3){ .x = sin(time), .y = cos(time) });
	normal.draw.hidden = frame_counter & 1;
	phys_sweep_point_against_mesh(&sweep_result, &solid.phys.mesh, &trace.phys.point, &(fvec3){ .y = 1 }, 11);
	trace.origin.position = sweep_result.path.end;
	normal.origin.position = sweep_result.hit_position;
	//normal.draw.hidden = normal.draw.hidden && sweep_result.hit_normal.y < 0;
	log_debug("normal " VEC3_FORMSPEC, VEC3_FORMSPEC_ARG(sweep_result.hit_normal));
	fvec4_quaternion_angle_delta(&normal.origin.quaternion, &sweep_result.hit_normal, &(fvec3){.y = -1});
	draw_buffer_display(buffer, &view, shader);
	ui_window_swap(window);
    }

    ui_window_destroy(window);
    draw_shader_program_free(shader);
    draw_buffer_free(buffer);

    return 0;
}
