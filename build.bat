@echo off
rem cl /nologo /Z7 /DEBUG /Fo"run_tree/ld38.obj" /Fe"run_tree/ld38.exe" /MT /EHsc src/ld38.cpp /link /nologo /subsystem:console kernel32.lib user32.lib gdi32.lib opengl32.lib advapi32.lib lib/SDL2.lib lib/SDL2_mixer.lib lib/glew32.lib
cl /nologo /O2 /DEBUG /Fo"run_tree/ld38.obj" /Fe"run_tree/ld38.exe" /MT /EHsc src/ld38.cpp /link /nologo /subsystem:windows kernel32.lib user32.lib gdi32.lib opengl32.lib advapi32.lib lib/SDL2.lib lib/SDL2_mixer.lib lib/glew32.lib
