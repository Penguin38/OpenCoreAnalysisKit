# Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

if [ -z "$BUILD_ANDROID_ABIS" ];then
    export BUILD_ANDROID_ABIS="arm64-v8a armeabi-v7a x86_64 x86"
fi
if [ -z $BUILD_TYPE ];then
    export BUILD_TYPE="Debug"
fi
if [ -z $BUILD_ANDROID_PLATFORM ];then
    export BUILD_ANDROID_PLATFORM="android-30"
fi
export BUILD_PRODUCT="aosp"
export BUILD_TARGET_PAGESIZE_4K="4K"
export BUILD_TARGET_PAGESIZE_16K="16K"
export BUILD_TARGET_PAGESIZE_ANDROID=$BUILD_TARGET_PAGESIZE_16K
export INSTALL_OUTPUT=output/$BUILD_PRODUCT/"$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

git submodule update --init --recursive

if [ $BUILD_PRODUCT == "aosp" ];then
if [ -z $ANDROID_NDK_HOME ];then
    echo "ANDROID_NDK_HOME is not set"
    echo "Example:"
    echo "    export ANDROID_NDK_HOME=NDK_DIR"
    echo "    ./script/build_android.sh"
    exit
fi
for CURRENT_ANDROID_ABI in $BUILD_ANDROID_ABIS
do
#build capstone
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$CURRENT_ANDROID_ABI \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_PLATFORM=$BUILD_ANDROID_PLATFORM \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      3rd-party/capstone/CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/capstone

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/capstone -j8

# build xz-utils
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$CURRENT_ANDROID_ABI \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_PLATFORM=$BUILD_ANDROID_PLATFORM \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DXZ_NLS=OFF \
      3rd-party/xz-utils/CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/xz-utils

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/xz-utils -j8

#build simpleini
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$CURRENT_ANDROID_ABI \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_PLATFORM=$BUILD_ANDROID_PLATFORM \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      3rd-party/simpleini/CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/simpleini

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/3rd-party/simpleini -j8

# build core-parser
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$CURRENT_ANDROID_ABI \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_PLATFORM=$BUILD_ANDROID_PLATFORM \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=android \
      -DCMAKE_BUILD_TARGET_PAGESIZE=$BUILD_TARGET_PAGESIZE_ANDROID \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/bin

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/bin -j8
done
fi
