@echo off
REM Call MSVC Environment

if "%CPUArch%"=="" for /F "tokens=1,2*" %%i in ('reg query "HKLM\System\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') do (
    if "%%i" =="PROCESSOR_ARCHITECTURE" (
        set CPUArch=%%k
    )
)

echo "CPU Arch : %CPUArch%"

if "%CPUArch%"="AMD64" goto AMD64
if "%CPUArch%"="x86" goto Intel32
echo "Yet this processor platform support %CPUArch%"
goto :EOF

:AMD64
goto VS140USE64BIT

:Intel32
if exist "%SystemRoot%\SysWOW64" goto AMD64
goto VS140USE32BIT

:ARM
if not exist "%VS140COMNTOOLS%..\..\VC\bin\x86_arm" goto VAILDVSNOTFOUD
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"  x86_arm
goto LauncherBuildTask

:ARM64
if not exist "%VS140COMNTOOLS%..\..\VC\bin\x86_arm64" goto VAILDVSNOTFOUD
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"  x86_arm64
goto LauncherBuildTask

goto :EOF



::64BIT Build

:VS140USE64BIT
if not exist "%VS140COMNTOOLS%..\..\VC\bin\x86_amd64" goto VS120USE64BIT
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"  x86_amd64
goto DomakeBuildTask

:VS120USE64BIT
IF not exist "%VS120COMNTOOLS%..\..\VC\bin\x86_amd64"  goto VS110USE64BIT
call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat"  x86_amd64
goto DomakeBuildTask

:VS110USE64BIT
if not exist "%VS110COMNTOOLS%..\..\VC\bin\x86_amd64"  goto VAILDVSNOTFOUD
call "%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"  x86_amd64
goto DomakeBuildTask


::X86 Build
:VS140USE32BIT
if not exist %VS140COMNTOOLS%  goto VS120USE32BIT
call "%VS140COMNTOOLS%\VsDevCmd.bat"
goto DomakeBuildTask

:VS120USE32BIT
IF not exist "%VS120COMNTOOLS%"  goto VS110USE32BIT
call "%VS120COMNTOOLS%.\VsDevCmd.bat"
goto DomakeBuildTask

:VS110USE32BIT
if not exist %VS110COMNTOOLS%  goto VAILDVSNOTFOUD
call "%VS110COMNTOOLS%\VsDevCmd.bat"
goto DomakeBuildTask

:VAILDVSNOTFOUD
echo "Not found valid VisualStudio"
goto :EOF

:DomakeBuildTask
PowerShell -NoProfile -NoLogo -ExecutionPolicy unrestricted -Command "[System.Threading.Thread]::CurrentThread.CurrentCulture = ''; [System.Threading.Thread]::CurrentThread.CurrentUICulture = '';& '%~dp0buildl.ps1' %*"
