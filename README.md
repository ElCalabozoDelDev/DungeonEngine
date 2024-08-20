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
Install dependencies
```
vcpkg install
```
Create build folder
```
mkdir build
cd build
```
Generate compiler files and compile
```
cmake .. -G "MinGW Makefiles"
make
```