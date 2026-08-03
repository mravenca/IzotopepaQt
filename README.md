# Izotopepa Complete Edition

A modern **2D platform game** and **reusable game engine** written in
**C++17** using **Qt 6**.

## Features

-   10 campaign levels
-   Environmental mechanics
-   Reusable enemy framework
-   Developer tools

# Building Izotopepa on Linux

This guide explains how to build **Izotopepa Complete Edition** on
Linux.

## Prerequisites

-   C++17 compiler
-   Qt 6 development packages
-   CMake
-   Ninja (recommended)
-   Git

### Fedora

``` bash
sudo dnf install git cmake ninja-build gcc-c++ qt6-qtbase-devel qt6-qttools-devel
```

## Clone

``` bash
git clone https://github.com/mravenca/IzotopepaQt.git
cd IzotopepaQt
```

## Debug build

``` bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/IzotopepaQtGameV3
```

## Release build

``` bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/IzotopepaQtGameV3
```

## Developer mode

``` bash
./build-release/IzotopepaQtGameV3 --level 7
./build-release/IzotopepaQtGameV3 --debug
./build-release/IzotopepaQtGameV3 --god
./build-release/IzotopepaQtGameV3 --level-file assets/levels/test.json
```

## Packaging

``` bash
mkdir -p dist/Izotopepa
cp build-release/IzotopepaQtGameV3 dist/Izotopepa/
cp build-release/IzotopepaLevelEditor dist/Izotopepa/
cp README.md LICENSE CHANGELOG.md dist/Izotopepa/
cp -r assets dist/Izotopepa/
tar -C dist -czf Izotopepa-linux-x86_64.tar.gz Izotopepa
```

## Troubleshooting

``` bash
ldd build-release/IzotopepaQtGameV3
```


# Building Izotopepa on Windows

This guide explains how to build **Izotopepa Complete Edition** on
Windows.

## Prerequisites

Install:

-   Qt 6 (MinGW 64-bit or MSVC 2022 64-bit)
-   Qt Creator
-   CMake
-   Ninja (recommended)
-   Git

> Do not mix MinGW Qt libraries with the MSVC compiler.

## Clone

``` powershell
git clone https://github.com/mravenca/IzotopepaQt.git
cd IzotopepaQt
```

## Build with Qt Creator (recommended)

1.  Open `CMakeLists.txt`.
2.  Select a Qt 6 Desktop kit.
3.  Choose **Release**.
4.  Build.

## Command line (MinGW)

``` bat
C:\Qt\6.x.x\mingw_64\bin\qt-cmake.bat ^
  -S . ^
  -B build-release ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-release
```

## Deploy

``` powershell
windeployqt --release build-release\IzotopepaQtGameV3.exe
windeployqt --release build-release\IzotopepaLevelEditor.exe
```

## Test

``` powershell
IzotopepaQtGameV3.exe
IzotopepaQtGameV3.exe --level 7
IzotopepaQtGameV3.exe --debug
IzotopepaQtGameV3.exe --god
IzotopepaLevelEditor.exe
```

## Package

Include:

-   IzotopepaQtGameV3.exe
-   IzotopepaLevelEditor.exe
-   Qt DLLs
-   platforms/qwindows.dll
-   assets/
-   README.md
-   LICENSE
-   CHANGELOG.md
