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
add_definitions(-D__${TARGET_BUILD_PRODUCT}_PARSER__)
add_definitions(-D__TARGET_PARSER__="${CMAKE_BUILD_PRODUCT}")

string(TOUPPER "${CMAKE_BUILD_TARGET}" TARGET_BUILD_PLATFORM)
add_definitions(-D__${TARGET_BUILD_PLATFORM}__)

string(TOUPPER "${CMAKE_BUILD_TYPE}" TARGET_BUILD_TYPE)
add_definitions(-D__PARSER_${TARGET_BUILD_TYPE}__)

string(TOUPPER "${CMAKE_BUILD_TARGET_PAGESIZE}" TARGET_BUILD_PAGESIZE)
#The program may have errors, but we still need to remove it because clang will no longer support it in the future.
#Remove code comments if necessary.
#set(AUTO_VAR_INIT_ZERO "-ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang")
if (TARGET_BUILD_TYPE STREQUAL "RELEASE")
set(OPTIMIZED_LEVEL "-O3")
else()
set(OPTIMIZED_LEVEL "-O2")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 ${AUTO_VAR_INIT_ZERO} ${OPTIMIZED_LEVEL} -fPIC")

set(CAPSTONE_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/../capstone)
set(CAPSTONE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/capstone/include)
add_library(capstone STATIC IMPORTED)
set_target_properties(capstone PROPERTIES IMPORTED_LOCATION ${CAPSTONE_LIB_DIR}/libcapstone.a)
include_directories(${CAPSTONE_INCLUDE_DIR})

if (NOT EXISTS "${CAPSTONE_LIB_DIR}/libcapstone.a")
message(STATUS "Not found ${CAPSTONE_LIB_DIR}/libcapstone.a")
else()
add_definitions(-D__CAPSTONE__)
set(CAPSTONE_LIB capstone)
endif()

include_directories(utils)
add_library(utils STATIC
            utils/base/utils.cpp
            utils/base/memory_map.cpp
            utils/logger/log.cpp
            utils/backtrace/callstack.cpp
            utils/zip/zip_file.cpp
            utils/zip/zip_entry.cpp)
if (TARGET_BUILD_PLATFORM STREQUAL "LINUX")
target_link_libraries(utils stdc++fs)
endif()

include_directories(core)
add_library(core STATIC
            core/api/core.cpp
            core/api/elf.cpp
            core/api/bridge.cpp
            core/api/unwind.cpp
            core/lp64/core.cpp
            core/lp32/core.cpp
            core/arm64/core.cpp
            core/arm64/unwind.cpp
            core/arm/core.cpp
            core/arm/unwind.cpp
            core/riscv64/core.cpp
            core/riscv64/unwind.cpp
            core/x86_64/core.cpp
            core/x86_64/unwind.cpp
            core/x86/core.cpp
            core/x86/unwind.cpp
            core/common/elf.cpp
            core/common/note_block.cpp
            core/common/load_block.cpp
            core/common/link_map.cpp
            core/common/native_frame.cpp
            core/common/disassemble/capstone.cpp)
target_link_libraries(core utils ${CAPSTONE_LIB})

include_directories(llvm)
add_library(llvm STATIC
            llvm/llvm.cpp
            llvm/cxx/string.cpp
            llvm/cxx/vector.cpp
            llvm/cxx/map.cpp
            llvm/cxx/list.cpp
            llvm/cxx/unordered_map.cpp
            llvm/cxx/deque.cpp
            llvm/cxx/split_buffer.cpp)
target_link_libraries(llvm core)

include_directories(android)
include_directories(android/art)
include_directories(android/jdk)
include_directories(android/sdk)
include_directories(android/services)
add_library(android STATIC
            android/android.cpp

            android/properties/prop_info.cpp
            android/properties/prop_area.cpp
            android/properties/property.cpp

            # art
            android/art/runtime/cache_helpers.cpp
            android/art/runtime/runtime.cpp
            android/art/runtime/art_field.cpp
            android/art/runtime/image.cpp
            android/art/runtime/thread_list.cpp
            android/art/runtime/thread.cpp
            android/art/runtime/class_linker.cpp
            android/art/runtime/indirect_reference_table.cpp
            android/art/runtime/vdex_file.cpp
            android/art/runtime/art_method.cpp
            android/art/runtime/stack.cpp
            android/art/runtime/managed_stack.cpp
            android/art/runtime/oat_quick_method_header.cpp
            android/art/runtime/oat.cpp
            android/art/runtime/nterp_helpers.cpp
            android/art/runtime/java_frame.cpp
            android/art/runtime/handle_scope.cpp
            android/art/runtime/monitor.cpp
            android/art/runtime/monitor_pool.cpp
            android/art/runtime/lock_word.cpp

            android/art/runtime/base/mutex.cpp
            android/art/runtime/base/locks.cpp

            android/art/runtime/mirror/object.cpp
            android/art/runtime/mirror/class.cpp
            android/art/runtime/mirror/array.cpp
            android/art/runtime/mirror/string.cpp
            android/art/runtime/mirror/dex_cache.cpp

            android/art/runtime/hprof/hprof.cpp

            android/art/runtime/gc/heap.cpp
            android/art/runtime/gc/space/space.cpp
            android/art/runtime/gc/space/fake_space.cpp
            android/art/runtime/gc/space/region_space.cpp
            android/art/runtime/gc/space/image_space.cpp
            android/art/runtime/gc/space/zygote_space.cpp
            android/art/runtime/gc/space/large_object_space.cpp
            android/art/runtime/gc/space/bump_pointer_space.cpp
            android/art/runtime/gc/accounting/space_bitmap.cpp

            android/art/runtime/jni/java_vm_ext.cpp
            android/art/runtime/oat/oat_file.cpp
            android/art/runtime/oat/stack_map.cpp
            android/art/runtime/interpreter/quick_frame.cpp
            android/art/runtime/interpreter/shadow_frame.cpp
            android/art/runtime/entrypoints/quick/callee_save_frame.cpp

            android/art/dex/primitive.cpp
            android/art/dex/modifiers.cpp
            android/art/dex/utf.cpp
            android/art/dex/descriptors_names.cpp
            android/art/dex/dex_file.cpp
            android/art/dex/dex_file_structs.cpp
            android/art/dex/standard_dex_file.cpp
            android/art/dex/compact_dex_file.cpp

            android/art/base/length_prefixed_array.cpp
            android/art/base/mem_map.cpp
            android/art/base/bit_memory_region.cpp
            android/art/base/bit_table.cpp

            android/art/dexdump/dexdump.cpp

            android/art/runtime/quick/quick_method_frame_info.cpp
            android/art/runtime/arch/arm/registers_arm.cpp
            android/art/runtime/arch/arm/quick_method_frame_info_arm.cpp
            android/art/runtime/arch/arm64/registers_arm64.cpp
            android/art/runtime/arch/arm64/quick_method_frame_info_arm64.cpp
            android/art/runtime/arch/x86/registers_x86.cpp
            android/art/runtime/arch/x86/quick_method_frame_info_x86.cpp
            android/art/runtime/arch/x86_64/registers_x86_64.cpp
            android/art/runtime/arch/x86_64/quick_method_frame_info_x86_64.cpp
            android/art/runtime/arch/riscv64/registers_riscv64.cpp
            android/art/runtime/arch/riscv64/quick_method_frame_info_riscv64.cpp

            android/art/runtime/jit/jit.cpp
            android/art/runtime/jit/jit_code_cache.cpp
            android/art/runtime/jit/jit_memory_region.cpp

            # jdk
            android/jdk/java/lang/Object.cpp
            android/jdk/java/lang/Class.cpp
            android/jdk/java/lang/Array.cpp
            android/jdk/java/lang/String.cpp
            android/jdk/java/lang/Throwable.cpp
            android/jdk/java/lang/StackTraceElement.cpp

            # sdk
            android/sdk/android/app/ContextImpl.cpp
            android/sdk/android/util/ArrayMap.cpp

            # services
            android/services/com/android/server/am/ActivityManagerService.cpp

            # logcat
            android/logcat/event_logtags.cpp
            android/logcat/LogBuffer.cpp
            android/logcat/SerializedLogBuffer.cpp
            android/logcat/SerializedData.cpp
            android/logcat/LogStatistics.cpp)
target_link_libraries(android core llvm)

set(LINKER_PTHREAD "")
if (TARGET_BUILD_PLATFORM STREQUAL "LINUX")
set(LINKER_PTHREAD "-lpthread")
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -ldl ${LINKER_PTHREAD}")

include_directories(parser)

add_library(parser STATIC
            parser/command/env.cpp
            parser/command/command.cpp
            parser/command/command_manager.cpp
            parser/command/help.cpp
            parser/command/cmd_time.cpp
            parser/command/cmd_core.cpp
            parser/command/cmd_exec.cpp
            parser/command/cmd_sysroot.cpp
            parser/command/cmd_mmap.cpp
            parser/command/cmd_auxv.cpp
            parser/command/cmd_file.cpp
            parser/command/cmd_linkmap.cpp
            parser/command/cmd_read.cpp
            parser/command/cmd_write.cpp
            parser/command/cmd_register.cpp
            parser/command/cmd_disassemble.cpp
            parser/command/cmd_thread.cpp
            parser/command/cmd_getprop.cpp
            parser/command/cmd_print.cpp
            parser/command/cmd_hprof.cpp
            parser/command/cmd_search.cpp
            parser/command/cmd_class.cpp
            parser/command/cmd_top.cpp
            parser/command/cmd_space.cpp
            parser/command/cmd_dex.cpp
            parser/command/cmd_method.cpp
            parser/command/cmd_logcat.cpp
            parser/command/cmd_dumpsys.cpp
            parser/command/cmd_env.cpp
            parser/command/cmd_shell.cpp
            parser/command/cmd_plugin.cpp
            parser/command/remote/cmd_remote.cpp
            parser/command/remote/opencore/opencore.cpp
            parser/command/remote/opencore/lp64/opencore.cpp
            parser/command/remote/opencore/lp32/opencore.cpp
            parser/command/remote/opencore/arm64/opencore.cpp
            parser/command/remote/opencore/arm/opencore.cpp
            parser/command/remote/opencore/x86_64/opencore.cpp
            parser/command/remote/opencore/x86/opencore.cpp
            parser/command/remote/opencore/riscv64/opencore.cpp
            parser/command/remote/hook/hook.cpp
            parser/command/fake/cmd_fake.cpp
            parser/command/fake/core/fake_core.cpp
            parser/command/fake/core/lp64/restore.cpp
            parser/command/fake/core/lp32/restore.cpp
            parser/command/fake/map/fake_map.cpp
            parser/command/fake/stack/fake_java_stack.cpp
            parser/command/backtrace/cmd_backtrace.cpp
            parser/command/backtrace/cmd_frame.cpp)
target_link_libraries(parser android llvm core utils)

add_executable(core-parser
               parser/ui/ui_thread.cpp
               parser/ui/mini_shell.cpp
               parser/work/work_thread.cpp
               parser/core_parser.cpp)
target_link_libraries(core-parser parser)
if (TARGET_BUILD_PAGESIZE STREQUAL "16K")
target_link_options(core-parser PRIVATE "-Wl,-z,max-page-size=16384")
endif()

add_executable(test tests/test.cpp)
target_link_libraries(test parser)

add_executable(cloctime tests/time.cpp)

add_library(plugin-simple SHARED
            parser/plugin/simple/simple.cpp)
target_link_libraries(plugin-simple parser)
set_target_properties(plugin-simple PROPERTIES PREFIX "")
