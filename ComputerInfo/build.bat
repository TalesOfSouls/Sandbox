REM Run this script ONLY in start_build_terminal.bat OR the vs developer x64 command prompt

cls

IF NOT EXIST ..\..\build mkdir ..\..\build
IF NOT EXIST ..\..\build\sandbox mkdir ..\..\build\sandbox

if not defined DevEnvDir (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - previous bat call failed.
    exit /b 1
)

cd ..\..\build\sandbox
del *.pdb > NUL 2> NUL
del *.idb > NUL 2> NUL
cd ..\..\Sandbox\ComputerInfo

REM Use /showIncludes for include debugging

REM Create main program
cl ^
    /MT /nologo /Gm- /GR- /EHsc /Od /Oi /WX /W4 /FC /Z7 /wd4201 ^
    /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /std:c++20 ^
    /D WIN32 /D _WINDOWS /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS ^
    /Fo"../../build/sandbox/" /Fe"../../build/sandbox/info_win32.exe" /Fd"../../build/sandbox/info_win32.pdb" /Fm"../../build/sandbox/info_win32.map" ^
    "info_win32.cpp"^
    /link /INCREMENTAL:no ^
    /SUBSYSTEM:CONSOLE /MACHINE:X64 ^
    kernel32.lib user32.lib gdi32.lib winmm.lib ^
    Advapi32.lib wbemuuid.lib iphlpapi.lib ws2_32.lib ^
    d3d12.lib dxgi.lib