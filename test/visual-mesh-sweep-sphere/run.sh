#!/bin/sh

if test -z "$MODEL"
then
    MODEL=src/gltf/test/load-test.glb
fi
    
#test/visual-mesh-traceline src/gltf/test/load-test.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
test/visual-mesh-sweep-sphere "$MODEL" src/physics-3d/test/visual-mesh-traceline/pointer.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
#test/visual-mesh-traceline src/gltf/test/load-test.glb src/gltf/test/load-test.glb src/draw-3d/test/test.vert src/draw-3d/test/test.frag
