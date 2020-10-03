@echo off

pushd bin
call cl /c /nologo /DHHU_USE_GLFW=1 /DGM_STATIC /Zi /I../include ../src/hogui/*.c
call link /lib *.obj /out:hogui.lib /nologo
popd