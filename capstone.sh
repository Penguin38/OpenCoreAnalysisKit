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

git submodule update --init --recursive

cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      capstone/CMakeLists.txt \
      -B $INSTALL_OUTPUT/linux/capstone

make -C $INSTALL_OUTPUT/linux/capstone -j8

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
      -DANDROID_PLATFORM=$BUILD_ANDROID_PLATFORM \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      capstone/CMakeLists.txt \
      -B $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/capstone

make -C $INSTALL_OUTPUT/android/$CURRENT_ANDROID_ABI/capstone -j8
done
fi
