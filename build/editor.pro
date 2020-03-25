#-------------------------------------------------
#
# Standalone Editor project.
# Qt-AdvancedDockingSystem.pro ->	.lib
# editor.pro ->						.exe
#
#-------------------------------------------------

TEMPLATE = subdirs

SUBDIRS += \
    ../src/editor/thirdparty/Qt-Advanced-Docking-System/AdvancedDockingSystem/Qt-Advanced-Docking-System.pro \
    editor

CONFIG += ordered

