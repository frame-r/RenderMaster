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
Open solution build/Engine.sln. Build yaml-cpp project first. Then build Engine project. Then the project Example can start the engine.
Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0.

Editor can be build in Qt.

![Alt text](editor1.png?raw=true "Example")
![Alt text](editor2.png?raw=true "Example")

## Keys
* ASWD - camera moving
* F - focus object
* hold Alt + left mouse - orbit mode, right - zoom
* F5/F6 - switch black/white theme
* F7 - reload all standard shaders


