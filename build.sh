# Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#export ANDROID_NDK=""
#export BUILD_TYPE="Release"
export BUILD_TYPE="Debug"
export BUILD_PRODUCT="aosp"
export INSTALL_OUTPUT=output/$BUILD_PRODUCT/"$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

cmake -DCMAKE_C_COMPILER="clang-12" \
      -DCMAKE_CXX_COMPILER="clang++-12" \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=linux \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/linux/bin

make -C $INSTALL_OUTPUT/linux/bin -j8

if [ $BUILD_PRODUCT == "aosp" ];then
if [ -z $ANDROID_NDK ];then
    echo "ANDROID_NDK is not set"
    echo "Example:"
    echo "    export ANDROID_NDK=NDK_DIR"
    echo "    ./build.sh"
    exit
fi
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI="arm64-v8a" \
      -DANDROID_NDK=$ANDROID_NDK \
      -DANDROID_PLATFORM=android-30 \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=android \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/bin

make -C $INSTALL_OUTPUT/android/bin -j8

cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI="x86_64" \
      -DANDROID_NDK=$ANDROID_NDK \
      -DANDROID_PLATFORM=android-30 \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=android \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/emulator/bin

make -C $INSTALL_OUTPUT/emulator/bin -j8
fi
