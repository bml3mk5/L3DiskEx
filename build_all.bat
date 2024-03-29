rem @echo off

set app=l3diskex
set path=%path%;"C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE";C:\Windows\Microsoft.NET\Framework\v4.0.30319

if not "%1"=="" set tag=/t:%1

svn update

cd "VC++2010"
: VCExpress.exe %app%.vcxproj %tag% /build Release

msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release;Platform=Win32
msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release;Platform=x64

cd ..
