# RenderMaster

Graphic engine for experiments with graphics and programming

## Features
PBR materials and textures, deferred shading, TAA, MSAA, reprojection, environment HDR cubemap, atmosphere, game objects hierarchy, manipulators, picking, imports assets, debug views, path tracing for preview.

## Planned
RTX, SSR, denoise, reflection probes, prefiltered reflections.


## Building
1) yaml-cpp.vcproj
2) Engine.vcproj
3) rmEd.pro (if you need editor. build in Qt)

Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0.


## Running
Editor can be run form Qt Designer or Visual Studio (Editor.vcproj).
Don't forget to set the environment variable PATH for your IDE: Engine.dll, FBX SDK dll, Qt binaries (only for editor).

![Alt text](preview.png?raw=true "Preview")
![Alt text](preview1.png?raw=true "Preview")


