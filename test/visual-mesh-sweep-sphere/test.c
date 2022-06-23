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
#include "../../sweep/moving-ellipsoid-against-mesh.h"
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
	phys_object_set_ellipsoid(&target->phys);
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

    draw_view view = { .quaternion = { .w = 1 }, .position.z = 5, .position.y = -2 };

    phys_sweep_result sweep_result;

    size_t frame_counter = 0;
    
    while (!ui_window_should_close(window))
    {
	frame_counter++;
	fvec time = ui_get_time();
	trace.origin.position = (fvec3){ .y = -10 };
	fvec4_setup_rotation_quaternion(&solid.origin.quaternion, &(fvec3){ .x = sin(time), .y = cos(time) });
	normal.draw.hidden = frame_counter & 1;
	trace.draw.hidden = !normal.draw.hidden;
	phys_sweep_ellipsoid_against_mesh(&sweep_result, &solid.phys.mesh, &trace.phys.ellipsoid, &(fvec3){ .y = 1 }, 11);
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



/*#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include "../../object/set.h"
#include "../../../convert/type.h"
#include "../../../convert/fd/source.h"
#include "../../../gltf/convert.h"
#include "../../../keyargs/keyargs.h"
#include "../../../game-interface/window.h"
#include "../../../game-interface/init.h"
#include "../../../game-interface/time.h"
#include "../../../range/alloc.h"
#include "../../../draw-3d/shader/load.h"
#include "../../../glad/glad.h"
#include "../../../draw-3d/mesh/def.h"
#include "../../../draw-3d/buffer/def.h"
#include "../../../draw-3d/buffer/draw.h"
#include "../../../draw-3d/buffer/loader.h"
#include "../../../window/alloc.h"
#include "../../../gltf/parse.h"
#include "../../../log/log.h"
#include "../../../thread/memory-pool.h"
#include "../../../thread/thread-pool.h"
#include "../../sweep/type.h"
#include "../../sweep/moving-ellipsoid-against-mesh.h"
#include "../../../thread/benchmark.h"

typedef struct {
    vec_object3 origin;
    draw_mesh_instance draw;
    phys_object phys;
}
    entity;

inline static void entity_init(entity * target, const glb * phys_mesh, draw_mesh * draw_mesh)
{
    *target = (entity){0};
    target->draw.origin = &target->origin;
    target->phys.origin = &target->origin;
    target->origin = VEC_OBJECT3_INITIALIZER;
    if (phys_mesh)
    {
	phys_object_set_mesh(&target->phys, phys_mesh);
    }
    else
    {
	phys_object_set_ellipsoid(&target->phys);
    }
    mesh_instance_set_mesh(&target->draw, draw_mesh);
}

thread_job_declare(draw_thread);
thread_job_define_arg(draw_thread,
		      struct
		      {
			  struct {
			      struct {
				  const char * vert;
				  const char * frag;
			      }
				  shader;
			      struct {
				  const char * solid;
				  const char * trace;
				  const char * normal;
			      }
				  mesh;
			  }
			      path;
		      });

void load_glb_path (glb * target, const char * path)
{
    static window_unsigned_char buffer = {0};
    fd_source fd_source = fd_source_init(open(path, O_RDONLY), &buffer);
    assert (fd_source.fd >= 0);
    assert (glb_load_from_source(target, &fd_source.source));
    //glb_toc_copy_mem(&target->toc);
    glb_copy_mem(target);
    convert_source_clear(&fd_source.source);
    window_rewrite(buffer);
}

void finish_frame (ui_window * window, draw_buffer * draw_buffer, draw_view * view, shader_program shader)
{
    draw_buffer_draw (draw_buffer, view, shader);
    ui_window_swap(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

thread_job_define_function(draw_thread)
{
    assert (ui_init());

    ui_window * window;

    assert (ui_window_new(&window, .width = 640, .height = 480, .title = "draw-3d test"));

    shader_program shader;

    //assert (shader_load_path(&shader, .vertex_path = argv[3], .fragment_path = argv[4]));
    assert (shader_load_path(&shader, .vertex_path = arg->path.shader.vert, .fragment_path = arg->path.shader.frag));
    
    window_unsigned_char file_buffer = {0};

    range_glb glb_range;
    range_calloc (glb_range, 3);

    load_glb_path (glb_range.begin, arg->path.mesh.solid);
    load_glb_path (glb_range.begin + 1, arg->path.mesh.trace);
    load_glb_path (glb_range.begin + 2, arg->path.mesh.normal);
    
    draw_buffer * draw_buffer;

    assert (draw_buffer_load_batch(&draw_buffer, &glb_range.alias_const));
    
    range_draw_mesh draw_meshes;
    draw_buffer_mesh_access(&draw_meshes, draw_buffer);

    entity solid;
    entity trace;
    entity normal;
    
    entity_init(&solid, glb_range.begin, draw_meshes.begin);
    entity_init(&trace, NULL, draw_meshes.begin + 1);
    entity_init(&normal, NULL, draw_meshes.begin + 2);

    window_clear(file_buffer);
    glb_clear(glb_range.begin);
    glb_clear(glb_range.begin + 1);
    glb_clear(glb_range.begin + 2);
    range_clear(glb_range);

    draw_view view = { .quaternion = { .w = 1 }, .position.z = 7, .position.y = -4 };

    fvec start_time = 0;
    
    fvec3 sweep_direction = { .y = 1 };

    phys_sweep_result sweep_result = {0};

    //fvec3 axis;

    fvec radius = 1;
    
    trace.origin.scale.x = radius;
    trace.origin.scale.y = 2 * radius;
    trace.origin.scale.z = radius;

    fvec3 normal_reference = { .y = -1 };

    size_t frame_counter = 0;

    while (!ui_window_should_close(window))
    {
	frame_counter++;
	
	//vec4_setup_rotation_quaternion(&view.quaternion, &(fvec3){ .y = start_time });

	fvec delta_time = ui_get_time() - start_time;
	start_time += delta_time;
	
	printf("%f fps\n", 1 / delta_time);

	benchmark_start();
	phys_sweep_ellipsoid_against_mesh(&sweep_result, &solid.phys.mesh, &trace.phys.ellipsoid, &sweep_direction, 14);
	benchmark_time("sweep");
	
	trace.origin.position = sweep_result.path.end;
	//normal.draw.hidden = true;
	//solid.draw.hidden = true;

	trace.draw.hidden = frame_counter & 1;
	//normal.draw.hidden = true;

	fvec4_quaternion_angle_delta(&normal.origin.quaternion, &sweep_result.hit_normal, &normal_reference);
	normal.origin.position = sweep_result.hit_position;

	//log_debug("normal " VEC3_FORMSPEC, VEC3_FORMSPEC_ARG(sweep_result.hit_normal));

	finish_frame(window, draw_buffer, &view, shader);

	solid.origin.position = (fvec3){ .x = 2 * sin(start_time) };
	//axis = (fvec3){ .x = delta_time, .y = delta_time * sin(start_time) };
	//vec4_apply_rotation_axis(&solid.origin.quaternion, &axis);
	fvec4_setup_rotation_quaternion(&solid.origin.quaternion, &(fvec3){ .x = sin(1.6180339 * start_time) });
	trace.origin.position = (fvec3){ .y = -10 };
	fvec3 trace_axis = { .y = start_time };
	fvec4_setup_rotation_quaternion(&trace.origin.quaternion, &trace_axis);
    }

    ui_window_destroy(window);

    draw_buffer_free(draw_buffer);
    
    thread_pool_quit(pool);
}


int main(int argc, char * argv[])
{
    draw_thread_job_memory_calloc_init();
    
    assert (argc == 6);
    
    draw_thread_job * root = draw_thread_job_memory_calloc();

    *draw_thread_job_init(root) = (draw_thread_job_arg)
    {
	.path.mesh.solid = argv[1],
	.path.mesh.trace = argv[2],
	.path.mesh.normal = argv[3],
	.path.shader.vert = argv[4],
	.path.shader.frag = argv[5],
    };

    log_debug("hosting");

    draw_thread_job_memory_unlock(root);
    
    thread_pool_host(4, draw_thread_job_generic(root));
    
    return 0;
}
*/
