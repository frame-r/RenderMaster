#-------------------------------------------------
#
# Deploy Editor project.
# Qt-AdvancedDockingSystem.pro ->	.lib
# editor.pro ->						[main_executable].exe
# installer.pro ->					[installer_executable].exe
#
#-------------------------------------------------
TEMPLATE = subdirs

SUBDIRS += \
    editor \
    ../src/editor/thirdparty/Qt-Advanced-Docking-System/AdvancedDockingSystem/Qt-Advanced-Docking-System.pro \
    installer

