@ECHO Off

ECHO Start unregistering engine...

rem x64 debug
ECHO Unregister engine x64... 
REG DELETE "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}" /f

rem x86 debug
ECHO Unregister engine x86... 
REG DELETE "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}" /f

rem common
ECHO Unregister CLSID... 
REG DELETE "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1\CLSID" /f
REG DELETE "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1" /f

pause