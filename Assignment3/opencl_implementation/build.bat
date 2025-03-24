@echo off
:: filepath: c:\Users\Zestro\Desktop\PDC\Assignment3\opencl_implementation\build.bat

:: Adjust these paths to match your OpenCL installation
set OPENCL_INC="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\include"
set OPENCL_LIB="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\lib\x64"

:: Compile
g++ -std=c++11 main.cpp -o edge_detector.exe -I%OPENCL_INC% -L%OPENCL_LIB% -lOpenCL

if %ERRORLEVEL% EQU 0 (
  echo Build successful!
) else (
  echo Build failed!
)

pause