test/phys-mesh-load: LDLIBS += -lm
test/phys-mesh-load: \
	src/convert/fd/source.o \
	src/convert/source.o \
	src/gltf/convert.o \
	src/gltf/env.o \
	src/gltf/gltf.o \
	src/json/json.o \
	src/log/log.o \
	src/physics/mesh/build.o \
	src/physics/mesh/load.o \
	src/physics/mesh/test/load.test.o \
	src/range/range_strdup_to_string.o \
	src/range/range_streq_string.o \
	src/range/streq.o \
	src/table/table.o \
	src/uri/uri.o \
	src/window/alloc.o \
	src/window/string.o \
	src/window/vprintf.o \
	src/window/vprintf_append.o \

phys-mesh-tests: test/phys-mesh-load
tests: phys-mesh-tests

C_PROGRAMS += test/phys-mesh-load
