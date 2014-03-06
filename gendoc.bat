@echo off
setlocal
pushd %~dp0

if not defined JSX_ROOT set JSX_ROOT=%CD%\..\jsx

set BUILDDOC="%JSX_ROOT%\build\doc\generate.bat"
if not exist %BUILDDOC% (
	echo %BUILDDOC% not found, please set JSX_ROOT environment variable
	exit /b -1
)

set JS_SRC_DIR=%CD%\rte
set CPP_SRC_DIR=%CD%\src
set DOCDIR=%CD%\doc\hydrogen

mkdir %DOCDIR% 2>nul
call %BUILDDOC% %JS_SRC_DIR% %CPP_SRC_DIR% %DOCDIR%

popd
endlocal