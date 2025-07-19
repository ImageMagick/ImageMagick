@echo off

set "SCRIPT_DIR=%~dp0"
set "EXECUTION_DIR=%cd%"

call "%SCRIPT_DIR%find-bash.cmd"

call set "PATH=%%SCRIPT_DIR:%EXECUTION_DIR%=%%"
call set "LINUX_PATH=%PATH:\=/%"

%BASH% -c "./%LINUX_PATH%/download-configure.sh"