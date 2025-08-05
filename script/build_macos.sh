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

if [ -z "$SUPPORT_CLANG_VERSIONS" ];then
    export SUPPORT_CLANG_VERSIONS="10 11 12 13 14 15 16 17 18"
fi
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
export INSTALL_OUTPUT=output/$BUILD_PRODUCT/"$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

git submodule update --init --recursive

# build capstone
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      3rd-party/capstone/CMakeLists.txt \
      -B $INSTALL_OUTPUT/macos/3rd-party/capstone

make -C $INSTALL_OUTPUT/macos/3rd-party/capstone -j8

# build xz-utils
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DXZ_NLS=OFF \
      3rd-party/xz-utils/CMakeLists.txt \
      -B $INSTALL_OUTPUT/macos/3rd-party/xz-utils

make -C $INSTALL_OUTPUT/macos/3rd-party/xz-utils -j8

# build core-parser
cmake -DCMAKE_BUILD_PRODUCT=$BUILD_PRODUCT \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_BUILD_TARGET=macos \
      -DCMAKE_BUILD_TARGET_PAGESIZE=$BUILD_TARGET_PAGESIZE_LINUX \
      CMakeLists.txt \
      -B $INSTALL_OUTPUT/macos/bin

make -C $INSTALL_OUTPUT/macos/bin -j8
