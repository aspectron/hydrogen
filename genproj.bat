@echo off
setlocal
pushd %~dp0

if not defined JSX_ROOT set JSX_ROOT=%CD%\..\jsx

set GENPROJ="%JSX_ROOT%\genproj.bat"
if not exist %GENPROJ% (
	echo %GENPROJ% not found, please set JSX_ROOT environment variable
	exit /b -1
)

call %GENPROJ% %CD%\hydrogen.gyp %JSX_ROOT%

popd
endlocal
