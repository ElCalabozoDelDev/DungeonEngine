# DungeonEngine

Requirements:
* Git
* vcpkg
* CMake
* Make
* a C++ compiler (I use Clang + LLVM)

clone the repository
```
git@github.com:ElCalabozoDelDev/DungeonEngine.git
cd DungeonEngine
```
Create build folder
```
mkdir build
cd build
```
Generate compiler files and compile
```
cmake .. --preset default 
make
```