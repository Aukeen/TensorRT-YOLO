@echo off
echo "Configuring..."

cmake "-DCMAKE_C_COMPILER=C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.40.33807/bin/Hostx64/x64/cl.exe" "-DCMAKE_CXX_COMPILER=C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.40.33807/bin/Hostx64/x64/cl.exe" -G "Visual Studio 17 2022" -S . -B build

echo "Building..."

cmake --build build --config Release -j 4
