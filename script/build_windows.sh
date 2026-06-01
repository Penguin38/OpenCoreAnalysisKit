# Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
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

if [ -z $BUILD_TYPE ];then
    export BUILD_TYPE="Debug"
fi
export BUILD_PRODUCT="aosp"
export BUILD_TARGET_PAGESIZE_4K="4K"
export BUILD_TARGET_PAGESIZE_16K="16K"
export BUILD_TARGET_PAGESIZE_LINUX=$BUILD_TARGET_PAGESIZE_4K
export INSTALL_OUTPUT=output/$BUILD_PRODUCT/"$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

git submodule update --init --recursive

if [ -z $WINDOWS_MINGW_HOME ];then
    echo "WINDOWS_MINGW_HOME is not set"
    echo "Example:"
    echo "    export WINDOWS_MINGW_HOME=MINGW_DIR"
    echo "    ./script/build_windows.sh"
    exit
fi

export BUILD_HOST_C_COMPILER="$WINDOWS_MINGW_HOME/bin/x86_64-w64-mingw32-clang"
export BUILD_HOST_CXX_COMPILER="$WINDOWS_MINGW_HOME/bin/x86_64-w64-mingw32-clang++"

# build capstone
cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      3rd-party/capstone/CMakeLists.txt \
      -B $INSTALL_OUTPUT/windows/3rd-party/capstone

make -C $INSTALL_OUTPUT/windows/3rd-party/capstone -j8

# build xz-utils
cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DXZ_NLS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
      3rd-party/xz-utils/CMakeLists.txt \
      -B $INSTALL_OUTPUT/windows/3rd-party/xz-utils

make -C $INSTALL_OUTPUT/windows/3rd-party/xz-utils -j8 liblzma

# build zstd
cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DZSTD_BUILD_SHARED=OFF \
      -DZSTD_BUILD_PROGRAMS=OFF \
      3rd-party/zstd/build/cmake/CMakeLists.txt \
      -B $INSTALL_OUTPUT/windows/3rd-party/zstd

make -C $INSTALL_OUTPUT/windows/3rd-party/zstd -j8

# build core-parser
cmake -DCMAKE_C_COMPILER=$BUILD_HOST_C_COMPILER \
      -DCMAKE_CXX_COMPILER=$BUILD_HOST_CXX_COMPILER \
      -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=windows \
      -DCMAKE_BUILD_TARGET_PAGESIZE=$BUILD_TARGET_PAGESIZE_LINUX \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/windows/bin

make -C $INSTALL_OUTPUT/windows/bin -j8
