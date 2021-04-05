# Overview
C++ SDK for running Runes.

# Build
Tested on macOS Big Sur 11.1.

## Prerequisites
- modern compiler with c++17 support (tested on Apple clang version 12.0.0 (clang-1200.0.32.29))
- cmake >= 3.15 (tested on 3.19.2)
- (optionally) tensorflow binary

## Guide
### Prepare repo
Cloning submodules might take some time as they fetch a lot of redundant data for things like tests.
```shell
git clone git@github.com:hotg-ai/rune_vm.git
cd rune_vm
git submodule update --init --recursive
```

### Configure via cmake
Notable cmake options - for complete list use cmake gui app:
- RUNE_VM_TFLITE_EXTERNAL - ON if you wish to use external tensorflow binary, e.g. installed via Pod. In that case you will have to provide RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS and RUNE_VM_INFERENCE_TFLITE_LIBRARIES too. OFF if you want tensorflow to be build as part of the project. OFF by default;
- CMAKE_BUILD_TYPE - set it to either Debug or Release;
- CMAKE_INSTALL_PREFIX - set to the location you wish the SDK to be installed to.

```shell
PROJECT_DIR=$(pwd)
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PROJECT_DIR/install
```

### Build
Notes: 
1. Tensorflow compilation via console tries to print pages for some reason. My console app - iTerm2 - blocks it after couple of requests, so if yours doesn't you may want to try it;
2. If BUILD_WORKERS_COUNT doesn't work on your platform, you may pass your count of cores or omit --parallel arg altogether;
```shell
BUILD_WORKERS_COUNT=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null)
cmake --build ./ --target rune_vm --parallel $BUILD_WORKERS_COUNT
```

### Install
```shell
cmake --build ./ --target install --parallel $BUILD_WORKERS_COUNT
```

### Test
*TBD*
