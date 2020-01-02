@echo off

pushd bin
call cl /I../include /Zi /nologo /MD ../*.c ../renderer/*.c ../hogui/*.c ../font/*.c /Fehogui.exe /link kernel32.lib opengl32.lib gdi32.lib user32.lib shell32.lib ../lib/glfw3.lib ../lib/freetype.lib
popd