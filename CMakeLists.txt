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

cmake_minimum_required(VERSION 3.21.1)
project(OpenCoreAnalysisKit)

string(TOUPPER "${CMAKE_BUILD_PRODUCT}" TARGET_BUILD_PRODUCT)
add_definitions(-D__${TARGET_BUILD_PRODUCT}__)

string(TOUPPER "${CMAKE_BUILD_TARGET}" TARGET_BUILD_PLATFORM)
add_definitions(-D__${TARGET_BUILD_PLATFORM}__)

set(AUTO_VAR_INIT_ZERO "-ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -Wno-unused-command-line-argument")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 ${AUTO_VAR_INIT_ZERO} -O2 -fPIC")

include_directories(utils)
add_library(utils STATIC
            utils/base/mem_map.cpp)

include_directories(core)
add_library(core STATIC
            core/api/core.cpp
            core/arm64/core.cpp
            core/arm/core.cpp
            core/riscv64/core.cpp
            core/x64/core.cpp
            core/x86/core.cpp)
target_link_libraries(core utils)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -ldl")
add_executable(test tests/test.cpp)
target_link_libraries(test core)
