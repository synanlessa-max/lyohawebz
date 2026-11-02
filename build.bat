@echo off
setlocal EnableDelayedExpansion
title Lobotomy build (x86)

rem ================================================================
rem  Visual Studio 2026 / 2022 compatible build script
rem  - finds MSBuild from VS 2026, VS 2022, or any VS via vswhere
rem  - uses the v143 toolset (VS 2022 compiler) if it is installed,
rem    otherwise switches the project to v145 - the VS 2026 compiler
rem ================================================================

set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"

rem === 1) find MSBuild: VS 2026 first, then VS 2022, then vswhere ===
set "MSBUILD="
for %%E in (Community Professional Enterprise BuildTools Insiders) do (
    if not defined MSBUILD (
        if exist "C:\Program Files\Microsoft Visual Studio\18\%%E\MSBuild\Current\Bin\MSBuild.exe" (
            set "MSBUILD=C:\Program Files\Microsoft Visual Studio\18\%%E\MSBuild\Current\Bin\MSBuild.exe"
        )
    )
)
if not defined MSBUILD (
    for %%E in (Community Professional Enterprise BuildTools) do (
        if not defined MSBUILD (
            if exist "C:\Program Files\Microsoft Visual Studio\2022\%%E\MSBuild\Current\Bin\MSBuild.exe" (
                set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\%%E\MSBuild\Current\Bin\MSBuild.exe"
            )
        )
    )
)
if not defined MSBUILD if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
)

if not defined MSBUILD (
    echo.
    echo  [ERROR] MSBuild not found!
    echo  Install Visual Studio 2026 or 2022 - free Community edition - with the
    echo  "Desktop development with C++" workload, then run this file again.
    echo.
    pause
    exit /b 1
)

echo Using MSBuild: "%MSBUILD%"

rem === 2) detect which compiler toolset is available ===
set "VSDIR="
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -prerelease -property installationPath`) do set "VSDIR=%%i"
)
if not defined VSDIR set "VSDIR=%MSBUILD:\MSBuild\Current\Bin\MSBuild.exe=%"

rem v143 = MSVC 14.3x-14.4x (VS 2022 compiler), v145 = MSVC 14.5x (VS 2026 compiler)
set "TOOLSET=v145"
if defined VSDIR if exist "!VSDIR!\VC\Tools\MSVC\" (
    for /d %%D in ("!VSDIR!\VC\Tools\MSVC\14.3*") do set "TOOLSET=v143"
    for /d %%D in ("!VSDIR!\VC\Tools\MSVC\14.4*") do set "TOOLSET=v143"
)

echo Using VS:      "!VSDIR!"
echo Compiler:      !TOOLSET!  - v143 is the VS 2022 compiler, v145 is the VS 2026 compiler
echo.
echo === Building Release x86 - 32-bit DLL for CS:GO ===
echo.

if "!TOOLSET!"=="v143" (
    rem VS 2022 compiler installed - project default toolset works as-is
    "%MSBUILD%" csgo-sdk.sln /p:Configuration=Release /p:Platform=x86 /m /nologo /v:m
) else (
    rem only the VS 2026 compiler is installed - override the project toolset
    "%MSBUILD%" csgo-sdk.sln /p:Configuration=Release /p:Platform=x86 /p:PlatformToolset=v145 /m /nologo /v:m
)

if errorlevel 1 (
    echo.
    echo  [ERROR] Build FAILED. Scroll up and read the error lines.
    echo  If the error says "The build tools for v143 cannot be found":
    echo  open Visual Studio Installer, click Modify, Individual components,
    echo  tick "MSVC v143 - VS 2022 C++ x64/x86 build tools", install and run again.
) else (
    echo.
    echo  [OK] BUILD SUCCESSFUL!
    echo  Your DLL is here:  release\fart.dll
    echo.
)
pause
