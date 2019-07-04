# RenderMaster

Graphic engine.

## Features
* Deferred render
* Physically based materials
* Image based lighting
* Temporal anti-aliasing
* Game objects hierarchy

### Editor
* Manipulators (Translate, Rotate)
* Picking
* Experimental Import FBX

## Building
1) yaml-cpp.vcproj
2) Engine.vcproj
3) Example.vcproj (if you need example)
4) rmEd.pro (if you need editor. build in Qt)

Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0.


## Running
Editor can be run form Qt Designer or Visual Studio (Editor.vcproj applied).
Don't forget to set the environment variable PATH within IDE for: Engine.dll, FBX SDK dll, Qt binaries (only for editor).

![Alt text](preview1.png?raw=true "Editor preview")
![Alt text](preview2.png?raw=true "Matte")
![Alt text](preview3.png?raw=true "Normal mapping")

## Keys
* ASWD - camera moving
* F - focus object
* hold Alt + left mouse - orbit mode, right - zoom
* F5/F6 - switch black/white theme
* F7 - reload all standard shaders

