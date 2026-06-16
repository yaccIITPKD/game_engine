@echo off
setlocal
pushd "%~dp0\.."

if not exist build mkdir build

cl .\bin\build.cpp ^
/nologo /Zi /Od /W4 /EHsc /permissive- ^
/Febuild\game.exe ^
/Fobuild\build.obj ^
/Fdbuild\game.pdb ^
/link /INCREMENTAL:NO shell32.lib || exit /b 1

cl .\game\game.cpp ^
/nologo /LD /Zi /Od ^
/Fobuild\game.obj ^
/Fdbuild\game_dll.pdb ^
/Fegame.dll ^
/link /INCREMENTAL:NO /NOIMPLIB || exit /b 1

del /Q build\*.obj build\*.exp build\*.ilk 2>nul