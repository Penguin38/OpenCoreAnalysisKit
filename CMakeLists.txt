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
            utils/base/utils.cpp
            utils/base/memory_map.cpp
            utils/logger/log.cpp
            utils/backtrace/callstack.cpp)
if (TARGET_BUILD_PLATFORM STREQUAL "LINUX")
target_link_libraries(utils stdc++fs)
endif()

include_directories(core)
add_library(core STATIC
            core/api/core.cpp
            core/lp64/core.cpp
            core/lp32/core.cpp
            core/arm64/core.cpp
            core/arm/core.cpp
            core/riscv64/core.cpp
            core/x64/core.cpp
            core/x86/core.cpp
            core/common/note_block.cpp
            core/common/load_block.cpp)
target_link_libraries(core utils)

include_directories(llvm)
add_library(llvm STATIC
            llvm/llvm.cpp
            llvm/cxx/string.cpp)
target_link_libraries(llvm core)

include_directories(android)
include_directories(android/art)
add_library(android STATIC
            android/android.cpp

            android/properties/prop_info.cpp
            android/properties/prop_area.cpp
            android/properties/property.cpp

            android/art/runtime/runtime.cpp
            android/art/runtime/art_field.cpp

            android/art/runtime/mirror/object.cpp
            android/art/runtime/mirror/class.cpp
            android/art/runtime/mirror/array.cpp
            android/art/runtime/mirror/string.cpp
            android/art/runtime/mirror/dex_cache.cpp

            android/art/runtime/hprof/hprof.cpp

            android/art/dex/primitive.cpp
            android/art/dex/modifiers.cpp
            android/art/dex/utf.cpp
            android/art/dex/descriptors_names.cpp
            android/art/dex/dex_file.cpp
            android/art/dex/dex_file_structs.cpp

            android/art/base/length_prefixed_array.cpp)
target_link_libraries(android core llvm)

set(LINKER_PTHREAD "")
if (TARGET_BUILD_PLATFORM STREQUAL "LINUX")
set(LINKER_PTHREAD "-lpthread")
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -ldl ${LINKER_PTHREAD}")

include_directories(parser)

add_library(parser STATIC
            parser/command/command_manager.cpp
            parser/command/help.cpp
            parser/command/command_manager.cpp
            parser/command/help.cpp
            parser/command/cmd_core.cpp
            parser/command/cmd_exec.cpp
            parser/command/cmd_sysroot.cpp
            parser/command/cmd_mmap.cpp
            parser/command/cmd_auxv.cpp
            parser/command/cmd_file.cpp
            parser/command/cmd_linkmap.cpp
            parser/command/cmd_read.cpp
            parser/command/cmd_write.cpp
            parser/command/cmd_getprop.cpp
            parser/command/cmd_print.cpp
            parser/command/cmd_hprof.cpp
            parser/command/cmd_env.cpp
            parser/command/cmd_shell.cpp
            parser/command/cmd_debug.cpp)
target_link_libraries(parser android llvm core utils)

add_executable(core-parser
               parser/ui/ui_thread.cpp
               parser/work/work_thread.cpp
               parser/core_parser.cpp)
target_link_libraries(core-parser parser)

add_executable(test tests/test.cpp)
target_link_libraries(test parser)

add_executable(cloctime tests/time.cpp)
