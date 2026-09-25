@echo off
rem Enter the x64 MSVC environment that tdesktop's docs/building-win.md asks for.
rem Expects SDK (Windows SDK version) and optionally VCVARS_VER (MSVC toolset version).
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSINSTALL="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
  echo Visual Studio with C++ tools was not found
  exit /b 1
)
set "VCVARS_ARGS=%SDK%"
if defined VCVARS_VER set "VCVARS_ARGS=%SDK% -vcvars_ver=%VCVARS_VER%"
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" %VCVARS_ARGS%
where cl >nul 2>nul || (
  echo vcvars64.bat %VCVARS_ARGS% did not set up cl.exe
  exit /b 1
)
