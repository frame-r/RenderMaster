call clean.bat

xcopy ..\bin\Engine_x64.dll installer\packages\com.framer.engine\data\bin\Engine_x64.dll* /C /Y /I
xcopy ..\bin\rmEd_x64.exe installer\packages\com.framer.engine\data\bin\rmEd_x64.exe* /C /Y /I

xcopy ..\bin\Theme.dat installer\packages\com.framer.engine\data\bin\Theme.dat* /C /Y /I
xcopy ..\bin\Windows.dat installer\packages\com.framer.engine\data\bin\Windows.dat* /C /Y /I
xcopy ..\bin\MainWindow.dat installer\packages\com.framer.engine\data\bin\MainWindow.dat* /C /Y /I

xcopy "C:\Program Files\Autodesk\FBX\FBX SDK\2019.0\lib\vs2015\x64\release\libfbxsdk.dll" installer\packages\com.framer.engine\data\bin\libfbxsdk.dll* /C /Y /I
xcopy C:\Windows\System32\msvcp140.dll installer\packages\com.framer.engine\data\bin\msvcp140.dll* /C /Y /I
xcopy C:\Windows\System32\vcruntime40.dll installer\packages\com.framer.engine\data\bin\vcruntime40.dll* /C /Y /I

C:\Qt\Qt5.12.3\5.12.3\msvc2017_64\bin\windeployqt.exe %~dp0\installer\packages\com.framer.engine\data\bin\rmEd_x64.exe --release --verbose 2

@echo Binaries completed for %~dp0installer\packages\com.framer.engine\data\bin

xcopy ..\resources installer\packages\com.framer.engine\data\resources /C /Y /E
@echo Resources completed for %~dp0installer\packages\com.framer.engine\data\resources