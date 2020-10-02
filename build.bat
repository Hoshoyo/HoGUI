@echo off

pushd bin
call cl /nologo /DHHU_USE_GLFW=1 /MDd /Zi /I../include ../src/*.c /Fehhu.exe /link kernel32.lib user32.lib ole32.lib shell32.lib gdi32.lib opengl32.lib ../lib/freetype.lib ../lib/glfw3.lib
popd