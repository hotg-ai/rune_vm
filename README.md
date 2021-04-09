# Overview
C++ SDK for running Runes.

# Build
Tested on macOS Big Sur 11.1.

## Prerequisites
- modern compiler with c++17 support (tested on Apple clang version 12.0.0 (clang-1200.0.32.29))
- cmake >= 3.15 (tested on 3.19.2)
- (optionally) tensorflow binary

### Android
- Modern ndk (e.g. 23)

### iOS
- Xcode

## Guide
### Prepare repo
Cloning submodules might take some time as they fetch a lot of redundant data for things like tests.
```shell
git clone git@github.com:hotg-ai/rune_vm.git
cd rune_vm
git submodule update --init --recursive
```

### Configure via cmake
#### Common options
Notable cmake options - for complete list use cmake gui app:
- RUNE_VM_TFLITE_EXTERNAL - ON if you wish to use external tensorflow binary, e.g. installed via Pod. In that case you will have to provide RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS and RUNE_VM_INFERENCE_TFLITE_LIBRARIES too. OFF if you want tensorflow to be build as part of the project. OFF by default;
- RUNE_VM_BUILD_TESTS - ON if you wish to build rune vm tests. ON by default;
- CMAKE_BUILD_TYPE - set it to either Debug or Release;
- CMAKE_INSTALL_PREFIX - set to the location you wish the SDK to be installed to.
- CMAKE_TOOLCHAIN_FILE - toolchain file which specifies a lot of internal options like compiler or linker. Usually required if you do cross compilation e.g. for mobile platform.

#### Mac or other host system
```shell
PROJECT_DIR=$(pwd)
CONFIG_POSTFIX=rel
BUILD_DIR=build-$CONFIG_POSTFIX
mkdir $BUILD_DIR && cd $BUILD_DIR
cmake ../ \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$PROJECT_DIR/install-$CONFIG_POSTFIX
```

#### Android
NOTE: Tests are not supported on Android.
For details see https://developer.android.com/ndk/guides/cmake .
- ANDROID_ABI - what arch to build for. Should match with target device. Possible values: armeabi-v7a armeabi-v7a with NEON	- same as -DANDROID_ABI=armeabi-v7a -DANDROID_ARM_NEON=ON, arm64-v8a, x86, x86_64; 
- ANDROID_NDK ;
- ANDROID_NATIVE_API_LEVEL ;
- ANDROID_STL ;
- ANDROID_TOOLCHAIN ;

Please pass path to NDK on your system to NDK_PATH variable in the script below.

```shell
PROJECT_DIR=$(pwd)
ABI=arm64-v8a
CONFIG_POSTFIX=android-rel-$ABI
BUILD_DIR=build-$CONFIG_POSTFIX
mkdir $BUILD_DIR && cd $BUILD_DIR
NDK_PATH=/Users/hotg_delimbetov/Library/Android/sdk/ndk/23.0.7196353
cmake ../ \
    -DRUNE_VM_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$PROJECT_DIR/install-$CONFIG_POSTFIX \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_ARM_NEON=ON \
    -DANDROID_NDK=$NDK_PATH \
    -DANDROID_NATIVE_API_LEVEL=28 \
    -DANDROID_STL=c++_shared \
    -DANDROID_TOOLCHAIN=clang \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake
```

#### iOS
NOTE: Tests are not supported on iOS.
If you use polly' toolchain, next env variables must be set:
- POLLY_IOS_DEVELOPMENT_TEAM must be set to the id of the team that will sign the binary. See https://stackoverflow.com/questions/18727894/how-can-i-find-my-apple-developer-team-id-and-team-agent-apple-id for details on how to find your id.
- POLLY_IOS_BUNDLE_IDENTIFIER - to the bundle identifier, e.g. "com.example.your_last_name_or_something".
iOS only supports external tensorflow at the moment. So you will have to specify DRUNE_VM_INFERENCE_TFLITE_* variables (see above for details). You should also specify PATH_TO_TFLITE_FRAMEWORK env variable if you wish to use script below.
```shell
PROJECT_DIR=$(pwd)
CONFIG_POSTFIX=ios-rel
BUILD_DIR=build-$CONFIG_POSTFIX
mkdir $BUILD_DIR && cd $BUILD_DIR
export POLLY_IOS_DEVELOPMENT_TEAM="YOUR_TEAM_ID"
export POLLY_IOS_BUNDLE_IDENTIFIER="BUNDLE_ID"
export PATH_TO_TFLITE_FRAMEWORK="PATH_ON_YOUR_SYSTEM"
cmake ../ \
    -GXcode \
    -DRUNE_VM_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$PROJECT_DIR/install-$CONFIG_POSTFIX \
    -DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/extern/polly/ios.cmake \
    -DRUNE_VM_TFLITE_EXTERNAL=ON \
    -DRUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS=$PATH_TO_TFLITE_FRAMEWORK/Headers \
    -DRUNE_VM_INFERENCE_TFLITE_LIBRARIES=$PATH_TO_TFLITE_FRAMEWORK/TensorFlowLiteC 
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
NOTE: RUNE_VM_BUILD_TESTS must be enabled during cmake configuration for tests to work.
```shell
./tests/rune_vm_tests
```
