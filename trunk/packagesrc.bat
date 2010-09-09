@echo off
del /q source\debug\*
del /q source\release\*
zip -r casimir-companion-src.zip source\* -x *.svn*
zip -r casimir-companion-src.zip script\* -x *.svn*
rem zip -r casimir-companion-src.zip LICENSE
rem zip -r casimir-companion-src.zip README.txt
