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

test/run-physics-3d-load: src/physics-3d/test/load-mesh-nodes/run.sh

SH_PROGRAMS += test/run-physics-3d-load
C_PROGRAMS += test/physics-3d-load

physics-3d-tests: test/run-physics-3d-load
physics-3d-tests: test/physics-3d-load

run-tests: run-physics-3d-tests
run-physics-3d-tests:
	DEPENDS=physics-3d-tests sh run-tests.sh test/run-physics-3d-load

depend: physics-3d-depend
physics-3d-depend:
	cdeps src/physics-3d > src/physics-3d/depends.makefile
