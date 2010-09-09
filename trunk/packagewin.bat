@echo off
rd /q /s windows
md windows
rem copy README.txt windows
rem copy LICENSE windows
copy source\release\digrec.exe windows
copy C:\Qt\2010.02.1\qt\bin\QtCore4.dll windows
copy C:\Qt\2010.02.1\qt\bin\QtGui4.dll windows
copy C:\Qt\2010.02.1\qt\bin\qextserialport1.dll windows
copy C:\Qt\2010.02.1\mingw\bin\mingwm10.dll windows
copy C:\Qt\2010.02.1\mingw\bin\libgcc_s_dw2-1.dll windows


cd windows
zip -r ..\casimir-companion-win.zip * -x *.svn*
cd ..
zip -r casimir-companion-win.zip script\* -x *.svn*