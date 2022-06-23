#!/bin/sh

if test -z "$MODEL"
then
    MODEL=src/gltf/test/load-test.glb
fi
    
#test/visual-mesh-traceline src/gltf/test/load-test.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
$DEBUG_PROGRAM test/visual-mesh-traceline "$MODEL" src/physics-3d/test/visual-mesh-traceline/pointer.glb src/physics-3d/test/visual-mesh-traceline/pointer.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
#test/visual-mesh-traceline src/gltf/test/load-test.glb src/gltf/test/load-test.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
