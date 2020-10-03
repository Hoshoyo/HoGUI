@echo off

pushd bin
call cl /nologo /Zi /MDd /DHHU_USE_GLFW /I../include ../src/*.c /Fehhu.exe /link kernel32.lib user32.lib ole32.lib shell32.lib gdi32.lib opengl32.lib ../lib/freetype.lib ../lib/glfw3.lib hogui.lib
popd