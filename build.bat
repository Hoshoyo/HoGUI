@echo off

pushd bin
call ml64 /nologo /c ../src/hogui/utils_masm.asm
call cl /c /nologo /DHHU_USE_GLFW=1 /DGM_STATIC /Zi /I../include ../src/hogui/*.c
call link /lib *.obj /out:hogui.lib
call cl /nologo /Zi /MDd /DHHU_USE_GLFW /I../include ../src/*.c /Fehhu.exe /link kernel32.lib user32.lib ole32.lib shell32.lib gdi32.lib opengl32.lib ../lib/freetype.lib ../lib/glfw3.lib hogui.lib
popd