# RenderMaster

Graphic engine.

## Features
* Deferred render
* Physically based materials
* Image based lighting
* Game objects system with parent-child relationship

### Editor
* Translate manupulator
* Picking
* Experimental Import FBX

## Building
Engine:
1) yaml-cpp.vcproj
2) Engine.vcproj
3) Example.vcproj (if you need example)

Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0.

Editor:

rmEd.pro build in Qt.


## Running
Editor can be run form Qt Designer or Visual Studio (Editor.vcproj applied).
Don't forget to set the environment variable PATH within IDE for: Engine, FBX SDK, Qt binaries (only for editor).

![Alt text](editor1.png?raw=true "Example")
![Alt text](editor2.png?raw=true "Example")

## Keys
* ASWD - camera moving
* F - focus object
* hold Alt + left mouse - orbit mode, right - zoom
* F5/F6 - switch black/white theme
* F7 - reload all standard shaders

