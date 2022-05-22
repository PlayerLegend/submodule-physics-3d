test/physics-3d-load: LDLIBS += -lm
test/physics-3d-load: \
	src/physics-3d/test/load-mesh-nodes/load.test.o \
	src/convert/source.o \
	src/convert/fd/source.o \
	src/gltf/convert.o \
	src/log/log.o \
	src/physics-3d/mesh/build.o \
	src/physics-3d/mesh/load.o \
	src/gltf/gltf.o \
	src/window/alloc.o \
	src/json/json.o \
	src/gltf/env.o \
	src/table/string.o \
	src/range/strdup_to_string.o \
	src/range/streq.o \
	src/range/strdup.o \
	src/range/string_init.o \
	src/range/alloc.o \

test/visual-mesh-traceline: CFLAGS += `pkg-config --cflags glfw3`
test/visual-mesh-traceline: LDLIBS += -lm -ldl `pkg-config --libs glfw3`
test/visual-mesh-traceline: \
	src/physics-3d/test/visual-mesh-traceline/test.o \
	src/game-interface/init.o \
	src/game-interface/window.o \
	src/game-interface/input.o \
	src/game-interface/time.o \
	src/log/log.o \
	src/glad/glad.o \
	src/draw-3d/buffer/loader.o \
	src/physics-3d/mesh/load.o \
	src/physics-3d/mesh/build.o \
	src/physics-3d/mesh/subset.o \
	src/physics-3d/set_hull.o \
	src/physics-3d/trace/line.o \
	src/physics-3d/trace/def.o \
	src/convert/source.o \
	src/convert/fd/source.o \
	src/gltf/convert.o \
	src/window/alloc.o \
	src/gltf/env.o \
	src/gltf/gltf.o \
	src/table/string.o \
	src/json/json.o \
	src/range/streq.o \
	src/range/strdup.o \
	src/range/string_init.o \
	src/range/strdup_to_string.o \
	src/draw-3d/buffer/def.o \
	src/draw-3d/mesh/def.o \
	src/draw-3d/shader/load.o \
	src/draw-3d/buffer/draw.o \
	src/vec/mat4.o \
	src/vec/vec3.o \
	src/vec/vec4.o \
	src/range/alloc.o \

test/run-visual-mesh-traceline: src/physics-3d/test/visual-mesh-traceline/run.sh

test/run-physics-3d-load: src/physics-3d/test/load-mesh-nodes/run.sh

SH_PROGRAMS += test/run-physics-3d-load
SH_PROGRAMS += test/run-visual-mesh-traceline
C_PROGRAMS += test/physics-3d-load
C_PROGRAMS += test/visual-mesh-traceline

physics-3d-tests: test/run-physics-3d-load
physics-3d-tests: test/physics-3d-load
physics-3d-tests: test/run-visual-mesh-traceline
physics-3d-tests: test/visual-mesh-traceline

run-tests: run-physics-3d-tests
run-physics-3d-tests:
	DEPENDS=physics-3d-tests sh run-tests.sh test/run-physics-3d-load

run-visual-mesh-traceline:
	make BUILD_ENV=debug test/run-visual-mesh-traceline test/visual-mesh-traceline
	$(DEBUG_PROGRAM) test/run-visual-mesh-traceline

depend: physics-3d-depend
physics-3d-depend:
	cdeps src/physics-3d > src/physics-3d/depends.makefile
