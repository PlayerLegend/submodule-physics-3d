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
	sh makedepend.sh src/physics-3d/physics-3d.makefile

# DO NOT DELETE

src/physics-3d/trace/sphere.o: src/vec/vec.h src/vec/vec3.h
src/physics-3d/trace/sphere.o: src/physics-3d/trace/def.h src/range/def.h
src/physics-3d/trace/sphere.o: src/window/def.h src/physics-3d/mesh/def.h
src/physics-3d/trace/line.o: src/vec/vec.h src/vec/vec3.h
src/physics-3d/trace/line.o: src/physics-3d/trace/def.h src/range/def.h
src/physics-3d/trace/line.o: src/window/def.h src/physics-3d/mesh/def.h
src/physics-3d/trace/line.o: src/physics-3d/trace/line.h src/log/log.h
src/physics-3d/ellipsoid/def.o: src/vec/vec.h src/vec/vec3.h
src/physics-3d/ellipsoid/def.o: src/physics-3d/trace/def.h
src/physics-3d/mesh/build.o: src/physics-3d/mesh/build.h src/vec/vec.h
src/physics-3d/mesh/build.o: src/vec/vec3.h src/range/def.h src/window/def.h
src/physics-3d/mesh/build.o: src/physics-3d/trace/def.h src/vec/range_vec3.h
src/physics-3d/mesh/build.o: src/window/alloc.h src/log/log.h
src/physics-3d/mesh/build.o: src/range/alloc.h
src/physics-3d/mesh/subset.o: src/vec/vec.h src/vec/vec3.h src/range/def.h
src/physics-3d/mesh/subset.o: src/window/def.h src/physics-3d/trace/def.h
src/physics-3d/mesh/subset.o: src/physics-3d/mesh/subset.h src/window/alloc.h
src/physics-3d/mesh/load.o: src/physics-3d/mesh/load.h
src/physics-3d/mesh/load.o: src/physics-3d/trace/def.h src/convert/status.h
src/physics-3d/mesh/load.o: src/convert/source.h src/gltf/def.h
src/physics-3d/mesh/load.o: src/gltf/convert.h src/gltf/env.h src/log/log.h
src/physics-3d/mesh/load.o: src/convert/fd/source.h
src/physics-3d/mesh/load.o: src/physics-3d/mesh/build.h src/vec/vec.h
src/physics-3d/mesh/load.o: src/vec/vec3.h src/range/def.h src/window/def.h
src/physics-3d/mesh/load.o: src/window/alloc.h src/gltf/parse.h
src/physics-3d/mesh/GLfloat3.o: src/vec/vec.h src/vec/vec3.h src/range/def.h
src/physics-3d/mesh/GLfloat3.o: src/window/def.h src/physics-3d/trace/def.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/physics-3d/mesh/load.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/physics-3d/trace/def.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/convert/status.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/convert/source.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/gltf/def.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/convert/fd/source.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/log/log.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/physics-3d/mesh/build.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/gltf/convert.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/gltf/parse.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/window/alloc.h
src/physics-3d/test/load-mesh-nodes/load.test.o: src/range/alloc.h
