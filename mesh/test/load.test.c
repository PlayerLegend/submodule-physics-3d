#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../../../log/log.h"
#include "../../../vec/vec.h"
#include "../../../vec/vec3.h"
#include "../../../range/def.h"
#include "../../../window/def.h"
#include "../def.h"
#include "../build.h"
#include "../../../convert/source.h"
#include "../../../uri/uri.h"
#include "../load.h"

int main(int argc, char * argv[])
{
    assert (argc == 2);

    convert_source * source = uri_open (NULL, "%s", argv[1]);

    range_phys_mesh_node nodes;
    
    if (!phys_mesh_node_load(&nodes, source))
    {
	log_fatal ("Failed to load from source");
    }

    convert_source_free (source);

    return 0;
    
fail:
    return 1;
}
