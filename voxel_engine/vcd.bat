if not exist build\CMakeFiles\debug\ (
  mkdir build\CMakeFiles\debug
  cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build\CMakeFiles\debug -G "Ninja"
)
cmake --build build\CMakeFiles\debug --target voxelcraft
cd build\debug
start /WAIT /B vc.exe
cd ..\..
