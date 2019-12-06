# RenderMaster

Graphic engine for experiments with graphics and programming

## Features
* Deferred render
* PBR
* TAA
* Game objects hierarchy

### Editor
* Manipulators (translate, rotate) + MSAA
* Picking
* Import .png, .jpg
* Experimental import .fbx

## Building
1) yaml-cpp.vcproj
2) Engine.vcproj
3) Example.vcproj (if you need example)
4) rmEd.pro (if you need editor. build in Qt)

Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0.


## Running
Editor can be run form Qt Designer or Visual Studio (Editor.vcproj applied).
Don't forget to set the environment variable PATH within IDE for: Engine.dll, FBX SDK dll, Qt binaries (only for editor).

![Alt text](preview.png?raw=true "Preview")

## Keys
* ASWD - camera moving
* F - focus object
* hold Alt + left mouse - orbit mode, right - zoom
* F5/F6 - switch black/white theme
* F7 - reload all standard shaders

