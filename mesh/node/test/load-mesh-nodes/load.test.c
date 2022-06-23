#include <unistd.h>
#include "../../load.h"
#include "../../../../../convert/type.h"
#include "../../../../../convert/source.h"
#include "../../../../../convert/fd/source.h"
#include "../../../../../gltf/load/source.h"
#include "../../../../../gltf/free.h"
#include "../../../../../log/log.h"
#include <assert.h>
#include "../../../../../window/alloc.h"
#include <stdlib.h>
#include "../../../../../range/alloc.h"

int main(int argc, char * argv[])
{
    assert (argc == 1);

    window_unsigned_char buffer = {0};

    fd_source fd_source = fd_source_init (STDIN_FILENO, &buffer);
    
    glb glb = {0};

    if (!glb_load_source(&glb, &fd_source.source))
    {
	log_fatal ("Failed to load glb from stdin");
    }

    range_phys_mesh_node mesh_nodes;

    assert (phys_mesh_node_load(&mesh_nodes, &glb));
    
    window_clear (buffer);
    glb_clear (&glb);

    range_clear(mesh_nodes);
    
    return 0;
    
fail:
    return 1;
}
