@ECHO Off

SETLOCAL

pushd %~dp0
set dir=%CD%
popd

rem SET "dir=%cd%"
:char
SET dir=%dir:~0,-1%
IF "%dir:~-1%"=="\" ( goto end ) else ( goto char )
:end
SET di



ECHO Start registering engine...

REM common
ECHO Register CLSID... 
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1" /f
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1\CLSID" /ve /t REG_SZ /d "{A889F560-58E4-11D0-A68A-0000837E3100}" /f
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1\CLSID" /v CLSID /t REG_SZ /d "{A889F560-58E4-11D0-A68A-0000837E3100}" /f

REM x64 debug
SET path_x_64_d=%dir%bin\x64\Debug\Engine.dll
SET path_dir_x_64_d=%dir%bin\x64\Debug
ECHO Registering engine x64 debug on %path_x_64_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}" /f /ve /t REG_SZ /d "Render Master 64" 
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /ve /t REG_SZ /d %path_x_64_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /v "ThreadingModel" /t REG_SZ /d "Both"
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /v "InstalledDir" /t REG_SZ /d %path_dir_x_64_d%

REM x86 debug
SET path_x_86_d=%dir%bin\Win32\Debug\Engine.dll
SET path_dir_x_86_d=%dir%bin\Win32\Debug
ECHO Registering engine x86 debug on %path_x_86_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}" /f /ve /t REG_SZ /d "Render Master 32 bit running on 64 bit system" 
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /ve /t REG_SZ /d %path_x_86_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /v "ThreadingModel" /t REG_SZ /d "Both"
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /v "InstalledDir" /t REG_SZ /d %path_dir_x_86_d%

pause