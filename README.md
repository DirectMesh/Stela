# Stela

Stela is a cross-platform game Engine Focused On Control By Having Scripts be in C++ And Very close to the Engine code. Stela Targets pretty much everything, Linux is the main development platform, it also supports MacOS, IOS, iPadOS, TvOS, VisionOS, and of course Windows.

Stela is not designed to be a beginner engine but is slowly being made to be more user-friendly. The current engine is in a very early state; this may change.

## Renders Supported By Stela:

1.  Vulkan
2.  Metal

## Building

To build Stela

### Windows

```
git clone --recursive https://github.com/DirectMesh/Stela
cd Stela
cmake -S . -B build
cmake --build build
```

The built engine is in the /bin folder.

### Linux

```
git clone --recursive https://github.com/DirectMesh/Stela
cd Stela
cmake -S . -B build
cmake --build build
```

The built engine is in /bin folder.

### MacOS

```
git clone --recursive https://github.com/DirectMesh/Stela
cd Stela
cmake -S . -B build
cmake --build build
```

The built engine is in /bin folder.

Or if you want to build an Xcode project


```
git clone --recursive https://github.com/DirectMesh/Stela
cd Stela
mkdir build
cd build
cmake -G Xcode ..
```

The Project is in the build folder.

### IOS or other Apple Platforms

```
git clone --recursive https://github.com/DirectMesh/Stela
cd Stela
mkdir build
cd build
cmake -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DIOS_PLATFORM=OS \
  ..
```

The Project is in the build folder.