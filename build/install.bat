@ECHO Off

SETLOCAL

pushd %~dp0
set dir=%CD%
popd

IF %dir:~-1%==\ SET dir=%dir:~0,-1%
IF %dir:~-6%==\build SET dir=%dir:~0,-6%
ECHO %dir%


ECHO Start registering engine...

rem SET "dir=%cd%"
rem :char
rem SET dir=%dir:~0,-1%
rem IF "%dir:~-1%"=="\" ( goto end ) else ( goto char )
rem :end
rem SET di


REM common
ECHO Register CLSID
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1" /f
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1\CLSID" /ve /t REG_SZ /d "{A889F560-58E4-11D0-A68A-0000837E3100}" /f
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderMaster.Component.1\CLSID" /v CLSID /t REG_SZ /d "{A889F560-58E4-11D0-A68A-0000837E3100}" /f


IF "%1"=="debug" GOTO Debug
IF "%1"=="release" GOTO Release
:Debug
	SET path_debug=Debug
GOTO End1
:Release
	SET path_debug=Release
:End1

REM x64
SET path_x_64_d=%dir%\bin\x64\%path_debug%\Engine.dll
SET path_dir_x_64_d=%dir%\bin\x64\%path_debug%
ECHO Registering engine x64 %path_debug% %path_x_64_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}" /f /ve /t REG_SZ /d "Render Master 64" 
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /ve /t REG_SZ /d %path_x_64_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /v "ThreadingModel" /t REG_SZ /d "Both"
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /v "WorkingDir" /t REG_SZ /d %path_dir_x_64_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{A889F560-58E4-11d0-A68A-0000837E3100}\InProcServer32" /f /v "InstalledDir" /t REG_SZ /d %dir%

REM x86
SET path_x_86_d=%dir%\bin\Win32\%path_debug%\Engine.dll
SET path_dir_x_86_d=%dir%\bin\Win32\%path_debug%
ECHO Registering engine x86 %path_debug% %path_x_86_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}" /f /ve /t REG_SZ /d "Render Master 32 bit running on 64 bit system" 
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /ve /t REG_SZ /d %path_x_86_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /v "ThreadingModel" /t REG_SZ /d "Both"
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /v "WorkingDir" /t REG_SZ /d %path_dir_x_86_d%
REG ADD "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Classes\CLSID\{A889F560-58E4-11D0-A68A-0000837E3100}\InProcServer32" /f /v "InstalledDir" /t REG_SZ /d %dir%

pause