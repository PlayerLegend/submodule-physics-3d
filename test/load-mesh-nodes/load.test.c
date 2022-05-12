#include "../../mesh/load.h"
#include <assert.h>
#include "../../../convert/fd/source.h"
#include <unistd.h>
#include "../../../log/log.h"
#include "../../mesh/build.h"
#include "../../../gltf/convert.h"
#include "../../../gltf/parse.h"
#include <stdio.h>
#include "../../../window/alloc.h"
#include "../../../range/alloc.h"

int main(int argc, char * argv[])
{
    assert (argc == 1);

    window_unsigned_char buffer = {0};

    fd_source fd_source = fd_source_init (STDIN_FILENO, &buffer);
    
    glb glb = {0};

    if (!glb_load_from_source(&glb, &fd_source.source))
    {
	log_fatal ("Failed to load glb from stdin");
    }

    window_phys_mesh_tri tris = {0};

    if (!phys_mesh_tri_load_glb(&tris, &glb))
    {
	log_fatal ("Failed to load tris from glb");
    }

    range_phys_mesh_node nodes = {0};
    
    phys_mesh_build(&nodes, &tris.region.alias_const);

    window_clear (buffer);
    window_clear (tris);
    range_clear (nodes);
    glb_clear (&glb);

    return 0;
    
fail:
    return 1;
}
