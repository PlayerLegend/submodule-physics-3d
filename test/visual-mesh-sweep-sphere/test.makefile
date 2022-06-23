test/visual-mesh-sweep-sphere: CFLAGS += `pkg-config --cflags glfw3`
test/visual-mesh-sweep-sphere: LDLIBS += -lm -ldl `pkg-config --libs glfw3` -lpthread
test/visual-mesh-sweep-sphere: \
	src/convert/fd/source.o \
	src/convert/source.o \
	src/draw-3d/buffer/display.o \
	src/draw-3d/buffer/free.o \
	src/draw-3d/buffer/index.o \
	src/draw-3d/buffer/load/glb.o \
	src/draw-3d/buffer/load/path.o \
	src/draw-3d/object/set.o \
	src/draw-3d/shader/free.o \
	src/draw-3d/shader/link.o \
	src/draw-3d/shader/load/path.o \
	src/draw-3d/shader/load/source.o \
	src/draw-3d/shader/load/text.o \
	src/game-interface/init.o \
	src/game-interface/input.o \
	src/game-interface/time.o \
	src/game-interface/window.o \
	src/glad/glad.o \
	src/gltf/copy.o \
	src/gltf/env.o \
	src/gltf/free.o \
	src/gltf/load/memory.o \
	src/gltf/load/path.o \
	src/gltf/load/source.o \
	src/json/json.o \
	src/log/log.o \
	src/physics-3d/mesh/boundary/set.o \
	src/physics-3d/mesh/free.o \
	src/physics-3d/mesh/load.o \
	src/physics-3d/mesh/node/load.o \
	src/physics-3d/object/set.o \
	src/physics-3d/sweep/moving-ellipsoid-against-mesh.o \
	src/physics-3d/test/visual-mesh-sweep-sphere/test.o \
	src/range/alloc.o \
	src/range/strdup.o \
	src/range/strdup_to_string.o \
	src/range/streq.o \
	src/range/string_init.o \
	src/table/string.o \
	src/thread/memory-pool.o \
	src/thread/thread-pool.o \
	src/vec/mat4.o \
	src/vec/object3.o \
	src/vec/shape3.o \
	src/vec/vec3.o \
	src/vec/vec4.o \
	src/window/alloc.o \

test/run-visual-mesh-sweep-sphere: src/physics-3d/test/visual-mesh-sweep-sphere/run.sh
SH_PROGRAMS += test/run-visual-mesh-sweep-sphere
C_PROGRAMS += test/visual-mesh-sweep-sphere

run-visual-mesh-sweep-sphere:
	make clean
	make depend
	make BUILD_ENV=debug test/run-visual-mesh-sweep-sphere test/visual-mesh-sweep-sphere
	test/run-visual-mesh-sweep-sphere
