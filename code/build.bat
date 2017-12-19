@echo off

REM mkdir "../build"
pushd "../build"

REM del "field_simulation.exe"
rem clang -Wno-writable-strings -O0 -g -gcodeview -o "field_simulation.exe" -I "../include" -I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0\include" -D DEBUG ../code/"main.cpp" -L"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v9.0/lib/x64" -lOpenCL.lib -lUser32.lib -lGdi32.lib -lopengl32.lib
cl /F 10000000 -MT -Zi -Od -Oi -nologo -TP /Fe"field_simulation.exe" -I "../include" -I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0\include" /DDEBUG ../code/"main.cpp" -link -LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0\lib\x64\" OpenCL.lib User32.lib Gdi32.lib OpenGL32.lib

if not errorlevel 1 (
   echo running...
   "field_simulation.exe"
)
popd
