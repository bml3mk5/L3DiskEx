@echo off
set app=l3diskex

call :copy_all_arch ".\VC++2010\Release" ".\bin\vc2010"

rem call :copy_one ".\ReleaseM" ".\bin\mingw32"

pause
goto eof

:copy_all_arch
call :copy_one %1\Win32 %2\Win32
call :copy_one %1\x64 %2\x64
goto eof

:copy_one
echo Copy %1\%app%.exe to %2 ...
if not exist %2 md %2
copy /b %1\%app%.exe %2
goto eof

:eof
