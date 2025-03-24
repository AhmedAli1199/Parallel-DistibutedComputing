# OpenCL Convolution Implementation

This project implements a 2D convolution operation using OpenCL.

## Building on Windows

### Prerequisites
- Visual Studio with C++ development tools
- OpenCL SDK (NVIDIA CUDA, AMD APP SDK, Intel OpenCL SDK, or similar)

### Build Instructions
1. Open a Visual Studio Developer Command Prompt
2. Navigate to this directory
3. Run the build script:
   ```
   build.bat
   ```

## Alternative Manual Compilation
If the build script doesn't work, you can compile manually with:

```
cl /nologo /W3 /O2 /EHsc /I"path\to\opencl\include" main.cpp /link /LIBPATH:"path\to\opencl\lib" OpenCL.lib
```

Replace "path\to\opencl\include" and "path\to\opencl\lib" with the actual paths on your system.

## Running the Program
After successful compilation, run the executable:
```
convolution.exe
```
