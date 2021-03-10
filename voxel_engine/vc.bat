mkdir build\CMakeFiles\release
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build\CMakeFiles\release -G "Ninja"
cmake --build build\CMakeFiles\release --target voxelcraft
cd build\release
start /WAIT /B vc.exe
cd ..\..
