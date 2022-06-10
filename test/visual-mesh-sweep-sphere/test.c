#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include "../../set_hull.h"
#include "../../mesh.h"
#include "../../sweep.h"
#include "../../object.h"
//#include "../../../vec/trace3.h"
#include "../../../convert/status.h"
#include "../../../convert/source.h"
#include "../../../convert/fd/source.h"
#include "../../../gltf/convert.h"
#include "../../../keyargs/keyargs.h"
#include "../../../game-interface/window.h"
#include "../../../game-interface/init.h"
#include "../../../game-interface/time.h"
#include "../../../range/alloc.h"
#include "../../../draw-3d/shader/def.h"
#include "../../../draw-3d/shader/load.h"
#include "../../../glad/glad.h"
#include "../../../draw-3d/mesh/def.h"
#include "../../../draw-3d/buffer/def.h"
#include "../../../draw-3d/buffer/draw.h"
#include "../../../draw-3d/buffer/loader.h"
#include "../../../window/alloc.h"
#include "../../../gltf/parse.h"
#include "../../../log/log.h"

void load_glb_path (glb * target, const char * path)
{
    static window_unsigned_char buffer = {0};
    fd_source fd_source = fd_source_init(open(path, O_RDONLY), &buffer);
    assert (fd_source.fd >= 0);
    assert (glb_load_from_source(target, &fd_source.source));
    glb_toc_copy_mem(&target->toc);
    convert_source_clear(&fd_source.source);
    window_rewrite(buffer);
}

int main(int argc, char * argv[])
{
    assert (argc == 5);
    
    assert (ui_init());

    ui_window * window;
    assert (ui_window_new(&window, .width = 640, .height = 480, .title = "draw-3d test"));
    assert (window);
    
    window_unsigned_char file_buffer = {0};

    range_glb glb_range;
    range_calloc (glb_range, 2);

    load_glb_path (glb_range.begin, argv[1]);
    load_glb_path (glb_range.begin + 1, argv[2]);

    draw_buffer * draw_buffer;
    assert (draw_buffer_load_batch(&draw_buffer, &glb_range.alias_const));
    assert (draw_buffer);

    range_draw_mesh draw_meshes;
    draw_buffer_mesh_access(&draw_meshes, draw_buffer);

    vec_object3 origin[2] =
    {
	VEC_OBJECT3_INITIALIZER,
	VEC_OBJECT3_INITIALIZER,
    };
    
    draw_mesh_instance instance[2] =
	{
	    { .origin = origin },
	    { .origin = origin + 1 },
	};
    
    mesh_instance_set_mesh(instance, draw_meshes.begin);
    mesh_instance_set_mesh(instance + 1, draw_meshes.begin + 1);

    draw_view view = { .quaternion = { .w = 1 }, .position.z = -10, .position.y = 1 };

    shader_program shader;
    assert (shader_load_path(&shader, .vertex_path = argv[3], .fragment_path = argv[4]));

    fvec3 axis = {0};

    double start_time = 0;
    double delta_time;

    phys_object object = { .origin = origin };

    assert (phys_object_set_hull(&object, glb_range.begin));
    
    float distance = 20;

    //phys_sweep sweep_result = { .path = { .vertex = { .x = 0, .y = 1.0 - distance, .z = 0 }, .direction = { 0, 1, 0 }, .distance = distance } };

    //phys_trace_result trace_result;
    fvec3_line trace = { .ray.vertex = { .x = 0, .y = 1.0 - distance, .z = 0 }, .ray.direction = { 0, 1, 0 }, .distance = distance };

    assert (!range_is_empty(object.mesh));

    phys_mesh_sweep sweep = {0};

    fvec3_aligned_ellipsoid shape = { .radius = { 1, 1, 1 } };

    //fvec3 initial_axis = { 1, 0, 0 }; vec4_apply_rotation_axis(&instance[0].origin->quaternion, &initial_axis);
    
    while (!ui_window_should_close(window))
    {
	draw_buffer_draw (draw_buffer, &view, shader);
	instance[0].origin->position = (fvec3){ .x = sin(start_time) };
	delta_time = ui_get_time() - start_time;
	start_time += delta_time;
	axis.z = delta_time;
	//axis.x = delta_time * sin(start_time);
	axis.x = delta_time;
	if (start_time == 0)
	{
	    continue;
	}
	
	vec4_apply_rotation_axis(&instance[0].origin->quaternion, &axis);

	phys_object_sweep_aligned_ellipsoid(&object, &sweep, &trace, &shape);

	//	phys_trace_line_object(&trace_result, &object, &trace);
	if (!sweep.result.is_hit)
	    log_debug("Distance %f hit %d", sweep.result.path.distance, sweep.result.is_hit);
        
	instance[1].origin->position = fvec3_line_point(&sweep.result.path, sweep.result.path.distance);

	assert (glGetError() == GL_NO_ERROR);
	ui_window_swap(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    ui_window_destroy(window);
    draw_buffer_free(draw_buffer);
    window_clear(file_buffer);

    gltf_clear(&glb_range.begin->gltf);
    gltf_clear(&(glb_range.begin + 1)->gltf);
    range_clear(glb_range);

    return 0;
}
