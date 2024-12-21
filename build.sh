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

#export ANDROID_NDK_HOME=""
#export BUILD_TYPE="Release"
export BUILD_ANDROID_ABIS="arm64-v8a armeabi-v7a x86_64 x86"
export SUPPORT_CLANG_VERSIONS="10 11 12 13 14 15 16 17 18"
for COMPILER_CLANG_VERSION in $SUPPORT_CLANG_VERSIONS
do
if command -v clang-$COMPILER_CLANG_VERSION &> /dev/null
then
export BUILD_HOST_C_COMPILER="clang-$COMPILER_CLANG_VERSION"
export BUILD_HOST_CXX_COMPILER="clang++-$COMPILER_CLANG_VERSION"
fi
done
if [ -z $BUILD_TYPE ];then
    export BUILD_TYPE="Debug"
fi
export BUILD_PRODUCT="aosp"
export BUILD_TARGET_PAGESIZE_4K="4K"
export BUILD_TARGET_PAGESIZE_16K="16K"
export BUILD_TARGET_PAGESIZE_LINUX=$BUILD_TARGET_PAGESIZE_4K
export BUILD_TARGET_PAGESIZE_ANDROID=$BUILD_TARGET_PAGESIZE_16K
export INSTALL_OUTPUT=output/$BUILD_PRODUCT/"$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

./capstone.sh

cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=linux \
      -DCMAKE_BUILD_TARGET_PAGESIZE=$BUILD_TARGET_PAGESIZE_LINUX \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/linux/bin

make -C $INSTALL_OUTPUT/linux/bin -j8

if [ $BUILD_PRODUCT == "aosp" ];then
if [ -z $ANDROID_NDK_HOME ];then
    echo "ANDROID_NDK_HOME is not set"
    echo "Example:"
    echo "    export ANDROID_NDK_HOME=NDK_DIR"
    echo "    ./build.sh"
    exit
fi
for CURRENT_ANDROID_ABI in $BUILD_ANDROID_ABIS
do
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$CURRENT_ANDROID_ABI \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_PLATFORM=android-30 \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=android \
      -DCMAKE_BUILD_TARGET_PAGESIZE=$BUILD_TARGET_PAGESIZE_ANDROID \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/bin

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/bin -j8
done
fi
