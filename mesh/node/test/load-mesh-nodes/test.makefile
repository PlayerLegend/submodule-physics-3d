test/physics-3d-load: LDLIBS += -lm
test/physics-3d-load: \
	src/convert/fd/source.o \
	src/convert/source.o \
	src/gltf/env.o \
	src/gltf/free.o \
	src/gltf/load/memory.o \
	src/gltf/load/source.o \
	src/json/json.o \
	src/log/log.o \
	src/physics-3d/mesh/node/load.o \
	src/physics-3d/mesh/node/test/load-mesh-nodes/load.test.o \
	src/range/alloc.o \
	src/range/strdup.o \
	src/range/strdup_to_string.o \
	src/range/streq.o \
	src/range/string_init.o \
	src/table/string.o \
	src/vec/shape3.o \
	src/vec/vec3.o \
	src/window/alloc.o \

test/run-physics-3d-load: src/physics-3d/mesh/node/test/load-mesh-nodes/run.sh
SH_PROGRAMS += test/run-physics-3d-load
C_PROGRAMS += test/physics-3d-load

physics-3d-tests: test/run-physics-3d-load
physics-3d-tests: test/physics-3d-load
tests: physics-3d-tests

run-tests: run-physics-3d-tests
run-physics-3d-tests: run-physics-3d-test-load-mesh-nodes
run-physics-3d-test-load-mesh-nodes:
	DEPENDS=physics-3d-tests sh run-tests.sh test/run-physics-3d-load
