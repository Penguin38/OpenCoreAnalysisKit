/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "ini.h"
#include "api/core.h"
#include "android.h"
#include "command/cmd_ini.h"
#include "properties/prop_info.h"
#include "properties/prop_area.h"
#include "logcat/LogStatistics.h"
#include "logcat/SerializedData.h"
#include "logcat/SerializedLogBuffer.h"
#include "fdtrack/fdtrack.h"
#include "unwindstack/Unwinder.h"
#include "native/android_BpBinder.h"
#include "native/android_os_BinderProxy.h"
#include "runtime/art_field.h"
#include "runtime/thread_list.h"
#include "runtime/oat/oat_file.h"
#include "runtime/managed_stack.h"
#include "runtime/interpreter/shadow_frame.h"
#include "runtime/monitor.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "runtime/mirror/dex_cache.h"
#include "runtime/gc/space/space.h"
#include "runtime/gc/space/region_space.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/gc/space/bump_pointer_space.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include "runtime/handle_scope.h"
#include "runtime/runtime.h"
#include "runtime/gc/heap.h"
#include "runtime/thread.h"
#include "runtime/image.h"
#include "runtime/class_linker.h"
#include "runtime/art_method.h"
#include "runtime/jni/java_vm_ext.h"
#include "runtime/jni/jni_env_ext.h"
#include "runtime/jni/local_reference_table.h"
#include "runtime/indirect_reference_table.h"
#include "runtime/jit/jit.h"
#include "runtime/jit/jit_code_cache.h"
#include "runtime/jit/jit_memory_region.h"
#include "runtime/oat/oat_file.h"
#include "runtime/oat/stack_map.h"
#include "runtime/base/mutex.h"
#include "runtime/base/locks.h"
#include "runtime/monitor.h"
#include "runtime/oat_quick_method_header.h"
#include "base/length_prefixed_array.h"
#include "base/mem_map.h"
#include "dex/standard_dex_file.h"
#include "dex/compact_dex_file.h"
#include "dex/dex_file_structs.h"
#include "dex/dex_file.h"
#include <unistd.h>
#include <getopt.h>
#include <filesystem>
#include <string>

#define INI_ENTRY(NAME) {#NAME, &NAME}
IniCommand* IniCommand::INSTANCE = nullptr;

IniCommand::IniCommand() : Command("ini") {
    INSTANCE = this;

    android_sections = {
        {"android_offsets", &android_offsets},
        {"android_sizes", &android_sizes},
        {"android_others", &android_others},
    };

    android_offsets = {
        // property
        INI_ENTRY(__PropInfo_offset__.serial),
        INI_ENTRY(__PropInfo_offset__.value),
        INI_ENTRY(__PropInfo_offset__.name),
        INI_ENTRY(__PropInfo_offset__.long_offset),
        INI_ENTRY(__PropArea_offset__.bytes_used_),
        INI_ENTRY(__PropArea_offset__.serial_),
        INI_ENTRY(__PropArea_offset__.magic_),
        INI_ENTRY(__PropArea_offset__.version_),
        INI_ENTRY(__PropArea_offset__.data_),
        INI_ENTRY(__Propbt_offset__.namelen),
        INI_ENTRY(__Propbt_offset__.prop),
        INI_ENTRY(__Propbt_offset__.left),
        INI_ENTRY(__Propbt_offset__.right),
        INI_ENTRY(__Propbt_offset__.children),
        INI_ENTRY(__Propbt_offset__.name),

        // logcat
        INI_ENTRY(__LogStatistics_offset__.mElements),
        INI_ENTRY(__SerializedData_offset__.data_),
        INI_ENTRY(__SerializedData_offset__.size_),
        INI_ENTRY(__SerializedLogBuffer_offset__.vtbl),
        INI_ENTRY(__SerializedLogBuffer_offset__.reader_list_),
        INI_ENTRY(__SerializedLogBuffer_offset__.tags_),
        INI_ENTRY(__SerializedLogBuffer_offset__.stats_),
        INI_ENTRY(__SerializedLogBuffer_offset__.max_size_),
        INI_ENTRY(__SerializedLogBuffer_offset__.logs_),

        // fdtrack
        INI_ENTRY(__FdEntry_offset__.backtrace),

        // unwindstack
        INI_ENTRY(__FrameData_offset__.pc),
        INI_ENTRY(__FrameData_offset__.function_name),
        INI_ENTRY(__FrameData_offset__.function_offset),

        // native binder
        INI_ENTRY(__BpBinder_offset__.mAlive),
        INI_ENTRY(__BpBinder_offset__.mObjects),
        INI_ENTRY(__BpBinder_offset__.mDescriptorCache),
        INI_ENTRY(__BpBinder_offset__.mTrackedUid),
        INI_ENTRY(__BinderProxyNativeData_offset__.mObject),
        INI_ENTRY(__BinderProxyNativeData_offset__.mOrgue),
        INI_ENTRY(__BinderProxyNativeData_offset__.mFrozenStateChangeCallbackList),

        // art
        INI_ENTRY(__ArtField_offset__.declaring_class_),
        INI_ENTRY(__ArtField_offset__.access_flags_),
        INI_ENTRY(__ArtField_offset__.field_dex_idx_),
        INI_ENTRY(__ArtField_offset__.offset_),
        INI_ENTRY(__LengthPrefixedArray_offset__.size_),
        INI_ENTRY(__LengthPrefixedArray_offset__.data_),
        INI_ENTRY(__ThreadList_offset__.list_),
        INI_ENTRY(__OatFile_offset__.vdex_),
        INI_ENTRY(__OatFile_offset__.begin_),
        INI_ENTRY(__OatFile_offset__.is_executable_),
        INI_ENTRY(__OatDexFile_offset__.oat_file_),
        INI_ENTRY(__OatDexFile_offset__.oat_class_offsets_pointer_),
        INI_ENTRY(__MemMap_offset__.name_),
        INI_ENTRY(__MemMap_offset__.begin_),
        INI_ENTRY(__MemMap_offset__.size_),
        INI_ENTRY(__ManagedStack_offset__.tagged_top_quick_frame_),
        INI_ENTRY(__ManagedStack_offset__.link_),
        INI_ENTRY(__ManagedStack_offset__.top_shadow_frame_),
        INI_ENTRY(__ShadowFrame_offset__.link_),
        INI_ENTRY(__ShadowFrame_offset__.method_),
        INI_ENTRY(__ShadowFrame_offset__.result_register_),
        INI_ENTRY(__ShadowFrame_offset__.dex_pc_ptr_),
        INI_ENTRY(__ShadowFrame_offset__.dex_instructions_),
        INI_ENTRY(__ShadowFrame_offset__.lock_count_data_),
        INI_ENTRY(__ShadowFrame_offset__.number_of_vregs_),
        INI_ENTRY(__ShadowFrame_offset__.dex_pc_),
        INI_ENTRY(__ShadowFrame_offset__.cached_hotness_countdown_),
        INI_ENTRY(__ShadowFrame_offset__.hotness_countdown_),
        INI_ENTRY(__ShadowFrame_offset__.frame_flags_),
        INI_ENTRY(__ShadowFrame_offset__.vregs_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.registers_size_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.ins_size_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.outs_size_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.tries_size_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.debug_info_off_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.insns_size_in_code_units_),
        INI_ENTRY(__StandardDexFile_CodeItem_offset__.insns_),
        INI_ENTRY(__CompactDexFile_CodeItem_offset__.fields_),
        INI_ENTRY(__CompactDexFile_CodeItem_offset__.insns_count_and_flags_),
        INI_ENTRY(__CompactDexFile_CodeItem_offset__.insns_),
        INI_ENTRY(__Monitor_offset__.monitor_lock_),
        INI_ENTRY(__Monitor_offset__.owner_),
        INI_ENTRY(__Monitor_offset__.obj_),
        INI_ENTRY(__Object_offset__.klass_),
        INI_ENTRY(__Object_offset__.monitor_),
        INI_ENTRY(__Class_offset__.class_loader_),
        INI_ENTRY(__Class_offset__.component_type_),
        INI_ENTRY(__Class_offset__.dex_cache_),
        INI_ENTRY(__Class_offset__.ext_data_),
        INI_ENTRY(__Class_offset__.iftable_),
        INI_ENTRY(__Class_offset__.name_),
        INI_ENTRY(__Class_offset__.super_class_),
        INI_ENTRY(__Class_offset__.vtable_),
        INI_ENTRY(__Class_offset__.ifields_),
        INI_ENTRY(__Class_offset__.methods_),
        INI_ENTRY(__Class_offset__.sfields_),
        INI_ENTRY(__Class_offset__.access_flags_),
        INI_ENTRY(__Class_offset__.class_flags_),
        INI_ENTRY(__Class_offset__.class_size_),
        INI_ENTRY(__Class_offset__.clinit_thread_id_),
        INI_ENTRY(__Class_offset__.dex_class_def_idx_),
        INI_ENTRY(__Class_offset__.dex_type_idx_),
        INI_ENTRY(__Class_offset__.num_reference_instance_fields_),
        INI_ENTRY(__Class_offset__.num_reference_static_fields_),
        INI_ENTRY(__Class_offset__.object_size_),
        INI_ENTRY(__Class_offset__.object_size_alloc_fast_path_),
        INI_ENTRY(__Class_offset__.primitive_type_),
        INI_ENTRY(__Class_offset__.reference_instance_offsets_),
        INI_ENTRY(__Class_offset__.status_),
        INI_ENTRY(__Class_offset__.copied_methods_offset_),
        INI_ENTRY(__Class_offset__.virtual_methods_offset_),
        INI_ENTRY(__Class_offset__.dex_cache_strings_),
        INI_ENTRY(__Class_offset__.direct_methods_),
        INI_ENTRY(__Class_offset__.virtual_methods_),
        INI_ENTRY(__Class_offset__.num_direct_methods_),
        INI_ENTRY(__Class_offset__.num_instance_fields_),
        INI_ENTRY(__Class_offset__.num_static_fields_),
        INI_ENTRY(__Class_offset__.num_virtual_methods_),
        INI_ENTRY(__Class_offset__.fields_),
        INI_ENTRY(__Class_offset__.embedded_vtable_length_),
        INI_ENTRY(__Class_offset__.imt_ptr_),
        INI_ENTRY(__Class_offset__.embedded_vtable_),
        INI_ENTRY(__String_offset__.count_),
        INI_ENTRY(__String_offset__.hash_code_),
        INI_ENTRY(__String_offset__.value_),
        INI_ENTRY(__String_offset__.value_compressed_),
        INI_ENTRY(__Array_offset__.length_),
        INI_ENTRY(__Array_offset__.first_element_),
        INI_ENTRY(__TypeId_offset__.descriptor_idx_),
        INI_ENTRY(__StringId_offset__.string_data_off_),
        INI_ENTRY(__FieldId_offset__.class_idx_),
        INI_ENTRY(__FieldId_offset__.type_idx_),
        INI_ENTRY(__FieldId_offset__.name_idx_),
        INI_ENTRY(__MethodId_offset__.class_idx_),
        INI_ENTRY(__MethodId_offset__.proto_idx_),
        INI_ENTRY(__MethodId_offset__.name_idx_),
        INI_ENTRY(__ProtoId_offset__.shorty_idx_),
        INI_ENTRY(__ProtoId_offset__.return_type_idx_),
        INI_ENTRY(__ProtoId_offset__.pad_),
        INI_ENTRY(__ProtoId_offset__.parameters_off_),
        INI_ENTRY(__TypeList_offset__.size_),
        INI_ENTRY(__TypeList_offset__.list_),
        INI_ENTRY(__TypeItem_offset__.type_idx_),
        INI_ENTRY(__Space_offset__.vtbl),
        INI_ENTRY(__Space_offset__.vtbl_GetType),
        INI_ENTRY(__Space_offset__.vtbl_IsDlMallocSpace),
        INI_ENTRY(__Space_offset__.vtbl_IsRosAllocSpace),
        INI_ENTRY(__Space_offset__.name_),
        INI_ENTRY(__ContinuousSpace_offset__.begin_),
        INI_ENTRY(__ContinuousSpace_offset__.end_),
        INI_ENTRY(__ContinuousSpace_offset__.limit_),
        INI_ENTRY(__ContinuousMemMapAllocSpace_offset__.live_bitmap_),
        INI_ENTRY(__ContinuousMemMapAllocSpace_offset__.mark_bitmap_),
        INI_ENTRY(__ContinuousMemMapAllocSpace_offset__.temp_bitmap_),
        INI_ENTRY(__DexFile_offset__.begin_),
        INI_ENTRY(__DexFile_offset__.size_),
        INI_ENTRY(__DexFile_offset__.data_begin_),
        INI_ENTRY(__DexFile_offset__.data_size_),
        INI_ENTRY(__DexFile_offset__.location_),
        INI_ENTRY(__DexFile_offset__.location_checksum_),
        INI_ENTRY(__DexFile_offset__.header_),
        INI_ENTRY(__DexFile_offset__.string_ids_),
        INI_ENTRY(__DexFile_offset__.type_ids_),
        INI_ENTRY(__DexFile_offset__.field_ids_),
        INI_ENTRY(__DexFile_offset__.method_ids_),
        INI_ENTRY(__DexFile_offset__.proto_ids_),
        INI_ENTRY(__DexFile_offset__.class_defs_),
        INI_ENTRY(__DexFile_offset__.method_handles_),
        INI_ENTRY(__DexFile_offset__.num_method_handles_),
        INI_ENTRY(__DexFile_offset__.call_site_ids_),
        INI_ENTRY(__DexFile_offset__.num_call_site_ids_),
        INI_ENTRY(__DexFile_offset__.hiddenapi_class_data_),
        INI_ENTRY(__DexFile_offset__.oat_dex_file_),
        INI_ENTRY(__DexFile_offset__.container_),
        INI_ENTRY(__DexFile_offset__.is_compact_dex_),
        INI_ENTRY(__Runtime_offset__.callee_save_methods_),
        INI_ENTRY(__Runtime_offset__.resolution_method_),
        INI_ENTRY(__Runtime_offset__.imt_conflict_method_),
        INI_ENTRY(__Runtime_offset__.imt_unimplemented_method_),
        INI_ENTRY(__Runtime_offset__.heap_),
        INI_ENTRY(__Runtime_offset__.monitor_pool_),
        INI_ENTRY(__Runtime_offset__.thread_list_),
        INI_ENTRY(__Runtime_offset__.class_linker_),
        INI_ENTRY(__Runtime_offset__.java_vm_),
        INI_ENTRY(__Runtime_offset__.jit_),
        INI_ENTRY(__Heap_offset__.continuous_spaces_),
        INI_ENTRY(__Heap_offset__.discontinuous_spaces_),
        INI_ENTRY(__Thread_offset__.tls32_),
        INI_ENTRY(__Thread_offset__.tlsPtr_),
        INI_ENTRY(__Thread_offset__.wait_mutex_),
        INI_ENTRY(__Thread_offset__.wait_cond_),
        INI_ENTRY(__Thread_offset__.wait_monitor_),
        INI_ENTRY(__Thread_tls_32bit_sized_values_offset__.state_and_flags),
        INI_ENTRY(__Thread_tls_32bit_sized_values_offset__.suspend_count),
        INI_ENTRY(__Thread_tls_32bit_sized_values_offset__.thin_lock_thread_id),
        INI_ENTRY(__Thread_tls_32bit_sized_values_offset__.tid),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.stack_end),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.managed_stack),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.jni_env),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.self),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.opeer),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.stack_begin),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.stack_size),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.monitor_enter_object),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.name),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.pthread_self),
        INI_ENTRY(__Thread_tls_ptr_sized_values_offset__.held_mutexes),
        INI_ENTRY(__ImageHeader_offset__.image_methods_),
        INI_ENTRY(__DexCache_offset__.class_loader_),
        INI_ENTRY(__DexCache_offset__.location_),
        INI_ENTRY(__DexCache_offset__.dex_file_),
        INI_ENTRY(__DexCache_offset__.preresolved_strings_),
        INI_ENTRY(__DexCache_offset__.resolved_call_sites_),
        INI_ENTRY(__DexCache_offset__.resolved_fields_),
        INI_ENTRY(__DexCache_offset__.resolved_method_types_),
        INI_ENTRY(__DexCache_offset__.resolved_methods_),
        INI_ENTRY(__DexCache_offset__.resolved_types_),
        INI_ENTRY(__DexCache_offset__.strings_),
        INI_ENTRY(__DexCache_offset__.num_preresolved_strings_),
        INI_ENTRY(__DexCache_offset__.num_resolved_call_sites_),
        INI_ENTRY(__DexCache_offset__.num_resolved_fields_),
        INI_ENTRY(__DexCache_offset__.num_resolved_method_types_),
        INI_ENTRY(__DexCache_offset__.num_resolved_methods_),
        INI_ENTRY(__DexCache_offset__.num_resolved_types_),
        INI_ENTRY(__DexCache_offset__.num_strings_),
        INI_ENTRY(__DexCache_offset__.resolved_fields_array_),
        INI_ENTRY(__DexCache_offset__.resolved_method_types_array_),
        INI_ENTRY(__DexCache_offset__.resolved_methods_array_),
        INI_ENTRY(__DexCache_offset__.resolved_types_array_),
        INI_ENTRY(__DexCache_offset__.strings_array_),
        INI_ENTRY(__RegionSpace_offset__.num_regions_),
        INI_ENTRY(__RegionSpace_offset__.regions_),
        INI_ENTRY(__RegionSpace_offset__.mark_bitmap_),
        INI_ENTRY(__Region_offset__.idx_),
        INI_ENTRY(__Region_offset__.live_bytes_),
        INI_ENTRY(__Region_offset__.begin_),
        INI_ENTRY(__Region_offset__.thread_),
        INI_ENTRY(__Region_offset__.top_),
        INI_ENTRY(__Region_offset__.end_),
        INI_ENTRY(__Region_offset__.objects_allocated_),
        INI_ENTRY(__Region_offset__.alloc_time_),
        INI_ENTRY(__Region_offset__.is_newly_allocated_),
        INI_ENTRY(__Region_offset__.is_a_tlab_),
        INI_ENTRY(__Region_offset__.state_),
        INI_ENTRY(__Region_offset__.type_),
        INI_ENTRY(__LargeObjectSpace_offset__.lock_),
        INI_ENTRY(__LargeObjectSpace_offset__.num_bytes_allocated_),
        INI_ENTRY(__LargeObjectSpace_offset__.num_objects_allocated_),
        INI_ENTRY(__LargeObjectSpace_offset__.total_bytes_allocated_),
        INI_ENTRY(__LargeObjectSpace_offset__.total_objects_allocated_),
        INI_ENTRY(__LargeObjectSpace_offset__.begin_),
        INI_ENTRY(__LargeObjectSpace_offset__.end_),
        INI_ENTRY(__LargeObjectMapSpace_offset__.large_objects_),
        INI_ENTRY(__LargeObject_offset__.mem_map),
        INI_ENTRY(__LargeObject_offset__.is_zygote),
        INI_ENTRY(__LargeObjectsPair_offset__.first),
        INI_ENTRY(__LargeObjectsPair_offset__.second),
        INI_ENTRY(__AllocationInfo_offset__.prev_free_),
        INI_ENTRY(__AllocationInfo_offset__.alloc_size_),
        INI_ENTRY(__FreeListSpace_offset__.mem_map_),
        INI_ENTRY(__FreeListSpace_offset__.allocation_info_map_),
        INI_ENTRY(__FreeListSpace_offset__.allocation_info_),
        INI_ENTRY(__FreeListSpace_offset__.free_end_),
        INI_ENTRY(__FreeListSpace_offset__.free_blocks_),
        INI_ENTRY(__BumpPointerSpace_offset__.main_block_size_),
        INI_ENTRY(__BumpPointerSpace_offset__.num_blocks_),
        INI_ENTRY(__BumpPointerSpace_offset__.block_sizes_),
        INI_ENTRY(__BumpPointerSpace_offset__.black_dense_region_size_),
        INI_ENTRY(__JavaVMExt_offset__.globals_),
        INI_ENTRY(__JavaVMExt_offset__.weak_globals_),
        INI_ENTRY(__JNIEnvExt_offset__.functions),
        INI_ENTRY(__JNIEnvExt_offset__.self_),
        INI_ENTRY(__JNIEnvExt_offset__.vm_),
        INI_ENTRY(__JNIEnvExt_offset__.locals_),
        INI_ENTRY(__IrtEntry_offset__.serial_),
        INI_ENTRY(__IrtEntry_offset__.references_),
        INI_ENTRY(__IndirectReferenceTable_offset__.segment_state_),
        INI_ENTRY(__IndirectReferenceTable_offset__.table_mem_map_),
        INI_ENTRY(__IndirectReferenceTable_offset__.table_),
        INI_ENTRY(__IndirectReferenceTable_offset__.kind_),
        INI_ENTRY(__IndirectReferenceTable_offset__.top_index_),
        INI_ENTRY(__LocalReferenceTable_offset__.segment_state_),
        INI_ENTRY(__LocalReferenceTable_offset__.tables_),
        INI_ENTRY(__LrtEntry_offset__.root_),
        INI_ENTRY(__ClassLinker_offset__.dex_caches_),
        INI_ENTRY(__DexCacheData_offset__.weak_root),
        INI_ENTRY(__DexCacheData_offset__.dex_file),
        INI_ENTRY(__DexCacheData_offset__.class_table),
        INI_ENTRY(__DexCacheData_offset__.registration_index),
        INI_ENTRY(__ArtMethod_offset__.declaring_class_),
        INI_ENTRY(__ArtMethod_offset__.access_flags_),
        INI_ENTRY(__ArtMethod_offset__.dex_code_item_offset_),
        INI_ENTRY(__ArtMethod_offset__.dex_method_index_),
        INI_ENTRY(__ArtMethod_offset__.method_index_),
        INI_ENTRY(__ArtMethod_offset__.hotness_count_),
        INI_ENTRY(__ArtMethod_offset__.imt_index_),
        INI_ENTRY(__ArtMethod_offset__.ptr_sized_fields_),
        INI_ENTRY(__PtrSizedFields_offset__.dex_cache_resolved_methods_),
        INI_ENTRY(__PtrSizedFields_offset__.data_),
        INI_ENTRY(__PtrSizedFields_offset__.entry_point_from_quick_compiled_code_),
        INI_ENTRY(__ContinuousSpaceBitmap_offset__.mem_map_),
        INI_ENTRY(__ContinuousSpaceBitmap_offset__.bitmap_begin_),
        INI_ENTRY(__ContinuousSpaceBitmap_offset__.bitmap_size_),
        INI_ENTRY(__ContinuousSpaceBitmap_offset__.heap_begin_),
        INI_ENTRY(__Jit_offset__.code_cache_),
        INI_ENTRY(__JitCodeCache_offset__.code_map_),
        INI_ENTRY(__JitCodeCache_offset__.exec_pages_),
        INI_ENTRY(__JitCodeCache_offset__.shared_region_),
        INI_ENTRY(__JitCodeCache_offset__.private_region_),
        INI_ENTRY(__JitCodeCache_offset__.jni_stubs_map_),
        INI_ENTRY(__JitCodeCache_offset__.method_code_map_),
        INI_ENTRY(__JitCodeCache_offset__.zygote_exec_pages_),
        INI_ENTRY(__JitCodeCache_offset__.zygote_map_),
        INI_ENTRY(__JniStubsMapPair_offset__.first),
        INI_ENTRY(__JniStubsMapPair_offset__.second),
        INI_ENTRY(__JniStubData_offset__.code_),
        INI_ENTRY(__JniStubData_offset__.methods_),
        INI_ENTRY(__ZygoteMap_offset__.array_),
        INI_ENTRY(__ZygoteMap_offset__.size_),
        INI_ENTRY(__ZygoteMap_offset__.region_),
        INI_ENTRY(__ZygoteMap_offset__.compilation_state_),
        INI_ENTRY(__JitMemoryRegion_offset__.exec_pages_),
        INI_ENTRY(__BaseMutex_offset__.vtbl),
        INI_ENTRY(__BaseMutex_offset__.vtbl_IsMutex),
        INI_ENTRY(__BaseMutex_offset__.vtbl_IsReaderWriterMutex),
        INI_ENTRY(__BaseMutex_offset__.vtbl_IsMutatorMutex),
        INI_ENTRY(__BaseMutex_offset__.name_),
        INI_ENTRY(__Mutex_offset__.exclusive_owner_),
        INI_ENTRY(__ReaderWriterMutex_offset__.state_),
        INI_ENTRY(__ReaderWriterMutex_offset__.exclusive_owner_),
        INI_ENTRY(__ConditionVariable_offset__.name_),
        INI_ENTRY(__ConditionVariable_offset__.guard_),
        INI_ENTRY(__ConditionVariable_offset__.sequence_),
        INI_ENTRY(__ConditionVariable_offset__.num_waiters_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.code_info_offset_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.data_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.vmap_table_offset_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.method_info_offset_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.frame_info_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.code_size_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.code_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.mapping_table_offset_),
        INI_ENTRY(__OatQuickMethodHeader_offset__.gc_map_offset_),
    };

    android_sizes = {
        INI_ENTRY(__SerializedLogBuffer_size__.THIS),
        INI_ENTRY(__SerializedLogBuffer_size__.vtbl),
        INI_ENTRY(__FdEntry_size__.THIS),
        INI_ENTRY(__FrameData_size__.THIS),
        INI_ENTRY(__ArtField_size__.THIS),
        INI_ENTRY(__Object_size__.THIS),
        INI_ENTRY(__Class_size__.THIS),
        INI_ENTRY(__String_size__.THIS),
        INI_ENTRY(__TypeId_size__.THIS),
        INI_ENTRY(__StringId_size__.THIS),
        INI_ENTRY(__FieldId_size__.THIS),
        INI_ENTRY(__MethodId_size__.THIS),
        INI_ENTRY(__ProtoId_size__.THIS),
        INI_ENTRY(__TypeItem_size__.THIS),
        INI_ENTRY(__HandleScope_size__.THIS),
        INI_ENTRY(__ImageHeader_size__.THIS),
        INI_ENTRY(__Region_size__.THIS),
        INI_ENTRY(__AllocationInfo_size__.THIS),
        INI_ENTRY(__IrtEntry_size__.THIS),
        INI_ENTRY(__LrtEntry_size__.THIS),
        INI_ENTRY(__ArtMethod_size__.THIS),
    };

    android_others = {
        INI_ENTRY(art::ImageHeader::kNumImageMethodsCount),
        INI_ENTRY(art::jni::kInitialLrtBytes),
        INI_ENTRY(art::jni::kSmallLrtEntries),
        INI_ENTRY(art::LockLevel::kMonitorLock),
        INI_ENTRY(art::LockLevel::kLockLevelCount),
        INI_ENTRY(art::CodeInfo::kNumHeaders),
        INI_ENTRY(art::CodeInfo::kNumBitTables),
        INI_ENTRY(art::StackMap::kNumStackMaps),
        INI_ENTRY(art::StackMap::kColNumKind),
        INI_ENTRY(art::StackMap::kColNumPackedNativePc),
        INI_ENTRY(art::StackMap::kColNumDexPc),
        INI_ENTRY(art::StackMap::kColNumRegisterMaskIndex),
        INI_ENTRY(art::StackMap::kColNumStackMaskIndex),
        INI_ENTRY(art::StackMap::kColNumInlineInfoIndex),
        INI_ENTRY(art::StackMap::kColNumDexRegisterMaskIndex),
        INI_ENTRY(art::StackMap::kColNumDexRegisterMapIndex),
        INI_ENTRY(art::RegisterMask::kNumRegisterMasks),
        INI_ENTRY(art::RegisterMask::kColNumValue),
        INI_ENTRY(art::RegisterMask::kColNumShift),
        INI_ENTRY(art::StackMask::kNumStackMasks),
        INI_ENTRY(art::StackMask::kColNumMask),
        INI_ENTRY(art::InlineInfo::kNumInlineInfos),
        INI_ENTRY(art::InlineInfo::kColNumIsLast),
        INI_ENTRY(art::InlineInfo::kColNumDexPc),
        INI_ENTRY(art::InlineInfo::kColNumMethodInfoIndex),
        INI_ENTRY(art::InlineInfo::kColNumArtMethodHi),
        INI_ENTRY(art::InlineInfo::kColNumArtMethodLo),
        INI_ENTRY(art::InlineInfo::kColNumNumberOfDexRegisters),
        INI_ENTRY(art::MethodInfo::kNumMethodInfos),
        INI_ENTRY(art::MethodInfo::kColNumMethodIndex),
        INI_ENTRY(art::MethodInfo::kColNumDexFileIndexKind),
        INI_ENTRY(art::MethodInfo::kColNumDexFileIndex),
        INI_ENTRY(art::DexRegisterMask::kNumDexRegisterMasks),
        INI_ENTRY(art::DexRegisterMask::kColNumMask),
        INI_ENTRY(art::DexRegisterMap::kNumDexRegisterMaps),
        INI_ENTRY(art::DexRegisterMap::kColNumCatalogueIndex),
        INI_ENTRY(art::DexRegisterInfo::kNumDexRegisterInfos),
        INI_ENTRY(art::DexRegisterInfo::kColNumKind),
        INI_ENTRY(art::DexRegisterInfo::kColNumPackedValue),
        INI_ENTRY(art::OatQuickMethodHeader::kIsCodeInfoMask),
        INI_ENTRY(art::OatQuickMethodHeader::kCodeInfoMask),
        INI_ENTRY(art::OatQuickMethodHeader::kCodeSizeMask),
    };

    Android::RegisterIniListener(IniCommand::ResetIni);

    static const char* kDwarfPathFilter[] = {
        "/apex/com.android.art/lib64/libart.so",
        "/apex/com.android.art/lib/libart.so",
        "/apex/com.android.art/lib64/libdexfile.so",
        "/apex/com.android.art/lib/libdexfile.so",
        "/apex/com.android.art/lib64/libartbase.so",
        "/apex/com.android.art/lib/libartbase.so",
        "/apex/com.android.runtime/lib64/libart.so",
        "/apex/com.android.runtime/lib/libart.so",
        "/apex/com.android.runtime/lib64/libdexfile.so",
        "/apex/com.android.runtime/lib/libdexfile.so",
        "/apex/com.android.runtime/lib64/libartbase.so",
        "/apex/com.android.runtime/lib/libartbase.so",
        "/system/lib64/libart.so",
        "/system/lib/libart.so",
    };

    CoreApi::RegisterSysRootListener([](LinkMap* map) {
        if (!INSTANCE) return;
        const char* name = map->name();
        bool match = false;
        for (const char* filter : kDwarfPathFilter) {
            if (!strcmp(name, filter)) {
                match = true;
                break;
            }
        }
        if (!match) return;

        auto& loader = map->GetDwarfLoader();
        if (!loader) return;
        int total = 0;
        loader->ForEachStruct([&](const dwarf::StructInfo& si) {
            total += INSTANCE->ApplyDwarfStruct(si);
            return false;
        });
        if (total > 0) {
            LOGI("DWARF: applied %d entries from %s\n", total, map->name());
            Android::Reset();
        }
    });
}

int IniCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    options.load = false;
    options.store = false;
    options.clear = false;
    options.dump_all = true;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"load",    no_argument, 0,  'l'},
        {"set",     no_argument, 0,  's'},
        {"clear",   no_argument, 0,  'c'},
        {0,         0,           0,   0 },
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "lsc",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'l':
                options.load = true;
                options.store = false;
                options.clear = false;
                options.dump_all = false;
                break;
            case 's':
                options.load = false;
                options.store = true;
                options.clear = false;
                options.dump_all = false;
                break;
            case 'c':
                options.load = false;
                options.store = false;
                options.clear = true;
                options.dump_all = false;
                break;
        }
    }
    options.optind = optind;

    return Command::CONTINUE;
}

int IniCommand::main(int argc, char* const argv[]) {
    if (options.dump_all) {
        ShowIniTable("android_offsets", android_offsets);
        ENTER();
        ShowIniTable("android_sizes", android_sizes);
        ENTER();
        ShowIniTable("android_others", android_others);
    } else if (options.load) {
        if (options.optind >= argc)
            return 0;

        Ini ini;
        bool reset = false;
        if (!ini.LoadFile(argv[options.optind])) {
            LOGE("Error loading %s file.\n", argv[options.optind]);
            return 0;
        }

        for (auto& section : android_sections) {
            if (!ini.HasSection(section.first.c_str()))
                continue;

            std::unordered_map<std::string, void *>* android_section = section.second;
            const char* section_name = section.first.c_str();

            for (auto& entry : *android_section) {
                const char* key_name = entry.first.c_str();
                const char* value = ini.GetValue(section_name, key_name, nullptr);
                if (value) {
                    uint32_t v = std::atoi(value);
                    if (v != OffsetValue(entry.second)) {
                        reset = true;
                        SetValue(entry.second, v);
                        LOGI("%s=%d\n", key_name, v);
                    }
                }
            }
        }

        if (reset) Android::Reset();
    } else if (options.store) {
        if (options.optind >= argc)
            return 0;

        std::unique_ptr<char[], void(*)(void*)> newcommand(strdup(argv[options.optind]), free);
        char *entry = strtok(newcommand.get(), "=");
        if (!entry) return 0;

        uint32_t v = 0x0;
        char *token = strtok(NULL, "=");
        if (token) v = std::atoi(token);

        std::string key = entry;
        bool reset = false;
        reset |= SetKeyValue(key, v, android_offsets);
        reset |= SetKeyValue(key, v, android_sizes);
        reset |= SetKeyValue(key, v, android_others);
        if (reset) Android::Reset();
    } else if (options.clear) {
        INSTANCE->values.clear();
        Android::Reset();
    }
    return 0;
}

struct DwarfBinding {
    const char* dwarf_class;
    const char* dwarf_member;   // nullptr for size binding
    void* target;
};

static const DwarfBinding kDwarfBindings[] = {
    // art::Runtime
    {"art::Runtime", "callee_save_methods_", &__Runtime_offset__.callee_save_methods_},
    {"art::Runtime", "resolution_method_", &__Runtime_offset__.resolution_method_},
    {"art::Runtime", "imt_conflict_method_", &__Runtime_offset__.imt_conflict_method_},
    {"art::Runtime", "imt_unimplemented_method_", &__Runtime_offset__.imt_unimplemented_method_},
    {"art::Runtime", "heap_", &__Runtime_offset__.heap_},
    {"art::Runtime", "monitor_pool_", &__Runtime_offset__.monitor_pool_},
    {"art::Runtime", "thread_list_", &__Runtime_offset__.thread_list_},
    {"art::Runtime", "class_linker_", &__Runtime_offset__.class_linker_},
    {"art::Runtime", "java_vm_", &__Runtime_offset__.java_vm_},
    {"art::Runtime", "jit_", &__Runtime_offset__.jit_},

    // art::gc::Heap
    {"art::gc::Heap", "continuous_spaces_", &__Heap_offset__.continuous_spaces_},
    {"art::gc::Heap", "discontinuous_spaces_", &__Heap_offset__.discontinuous_spaces_},

    // art::Thread
    {"art::Thread", "tls32_", &__Thread_offset__.tls32_},
    {"art::Thread", "tlsPtr_", &__Thread_offset__.tlsPtr_},
    {"art::Thread", "wait_mutex_", &__Thread_offset__.wait_mutex_},
    {"art::Thread", "wait_cond_", &__Thread_offset__.wait_cond_},
    {"art::Thread", "wait_monitor_", &__Thread_offset__.wait_monitor_},

    // art::Thread::tls_32bit_sized_values
    {"art::Thread::tls_32bit_sized_values", "state_and_flags", &__Thread_tls_32bit_sized_values_offset__.state_and_flags},
    {"art::Thread::tls_32bit_sized_values", "suspend_count", &__Thread_tls_32bit_sized_values_offset__.suspend_count},
    {"art::Thread::tls_32bit_sized_values", "thin_lock_thread_id", &__Thread_tls_32bit_sized_values_offset__.thin_lock_thread_id},
    {"art::Thread::tls_32bit_sized_values", "tid", &__Thread_tls_32bit_sized_values_offset__.tid},

    // art::Thread::tls_ptr_sized_values
    {"art::Thread::tls_ptr_sized_values", "stack_end", &__Thread_tls_ptr_sized_values_offset__.stack_end},
    {"art::Thread::tls_ptr_sized_values", "managed_stack", &__Thread_tls_ptr_sized_values_offset__.managed_stack},
    {"art::Thread::tls_ptr_sized_values", "jni_env", &__Thread_tls_ptr_sized_values_offset__.jni_env},
    {"art::Thread::tls_ptr_sized_values", "self", &__Thread_tls_ptr_sized_values_offset__.self},
    {"art::Thread::tls_ptr_sized_values", "opeer", &__Thread_tls_ptr_sized_values_offset__.opeer},
    {"art::Thread::tls_ptr_sized_values", "stack_begin", &__Thread_tls_ptr_sized_values_offset__.stack_begin},
    {"art::Thread::tls_ptr_sized_values", "stack_size", &__Thread_tls_ptr_sized_values_offset__.stack_size},
    {"art::Thread::tls_ptr_sized_values", "monitor_enter_object", &__Thread_tls_ptr_sized_values_offset__.monitor_enter_object},
    {"art::Thread::tls_ptr_sized_values", "name", &__Thread_tls_ptr_sized_values_offset__.name},
    {"art::Thread::tls_ptr_sized_values", "pthread_self", &__Thread_tls_ptr_sized_values_offset__.pthread_self},
    {"art::Thread::tls_ptr_sized_values", "held_mutexes", &__Thread_tls_ptr_sized_values_offset__.held_mutexes},

    // art::ThreadList
    {"art::ThreadList", "list_", &__ThreadList_offset__.list_},

    // art::ClassLinker
    {"art::ClassLinker", "dex_caches_", &__ClassLinker_offset__.dex_caches_},

    // art::ClassLinker::DexCacheData
    {"art::ClassLinker::DexCacheData", "weak_root", &__DexCacheData_offset__.weak_root},
    {"art::ClassLinker::DexCacheData", "dex_file", &__DexCacheData_offset__.dex_file},
    {"art::ClassLinker::DexCacheData", "class_table", &__DexCacheData_offset__.class_table},
    {"art::ClassLinker::DexCacheData", "registration_index", &__DexCacheData_offset__.registration_index},

    // art::mirror::Object
    {"art::mirror::Object", nullptr, &__Object_size__.THIS},
    {"art::mirror::Object", "klass_", &__Object_offset__.klass_},
    {"art::mirror::Object", "monitor_", &__Object_offset__.monitor_},

    // art::mirror::Class
    {"art::mirror::Class", nullptr, &__Class_size__.THIS},
    {"art::mirror::Class", "class_loader_", &__Class_offset__.class_loader_},
    {"art::mirror::Class", "component_type_", &__Class_offset__.component_type_},
    {"art::mirror::Class", "dex_cache_", &__Class_offset__.dex_cache_},
    {"art::mirror::Class", "ext_data_", &__Class_offset__.ext_data_},
    {"art::mirror::Class", "iftable_", &__Class_offset__.iftable_},
    {"art::mirror::Class", "name_", &__Class_offset__.name_},
    {"art::mirror::Class", "super_class_", &__Class_offset__.super_class_},
    {"art::mirror::Class", "vtable_", &__Class_offset__.vtable_},
    {"art::mirror::Class", "ifields_", &__Class_offset__.ifields_},
    {"art::mirror::Class", "methods_", &__Class_offset__.methods_},
    {"art::mirror::Class", "sfields_", &__Class_offset__.sfields_},
    {"art::mirror::Class", "access_flags_", &__Class_offset__.access_flags_},
    {"art::mirror::Class", "class_flags_", &__Class_offset__.class_flags_},
    {"art::mirror::Class", "class_size_", &__Class_offset__.class_size_},
    {"art::mirror::Class", "clinit_thread_id_", &__Class_offset__.clinit_thread_id_},
    {"art::mirror::Class", "dex_class_def_idx_", &__Class_offset__.dex_class_def_idx_},
    {"art::mirror::Class", "dex_type_idx_", &__Class_offset__.dex_type_idx_},
    {"art::mirror::Class", "num_reference_instance_fields_", &__Class_offset__.num_reference_instance_fields_},
    {"art::mirror::Class", "num_reference_static_fields_", &__Class_offset__.num_reference_static_fields_},
    {"art::mirror::Class", "object_size_", &__Class_offset__.object_size_},
    {"art::mirror::Class", "object_size_alloc_fast_path_", &__Class_offset__.object_size_alloc_fast_path_},
    {"art::mirror::Class", "primitive_type_", &__Class_offset__.primitive_type_},
    {"art::mirror::Class", "reference_instance_offsets_", &__Class_offset__.reference_instance_offsets_},
    {"art::mirror::Class", "status_", &__Class_offset__.status_},
    {"art::mirror::Class", "copied_methods_offset_", &__Class_offset__.copied_methods_offset_},
    {"art::mirror::Class", "virtual_methods_offset_", &__Class_offset__.virtual_methods_offset_},
    {"art::mirror::Class", "dex_cache_strings_", &__Class_offset__.dex_cache_strings_},
    {"art::mirror::Class", "direct_methods_", &__Class_offset__.direct_methods_},
    {"art::mirror::Class", "virtual_methods_", &__Class_offset__.virtual_methods_},
    {"art::mirror::Class", "num_direct_methods_", &__Class_offset__.num_direct_methods_},
    {"art::mirror::Class", "num_instance_fields_", &__Class_offset__.num_instance_fields_},
    {"art::mirror::Class", "num_static_fields_", &__Class_offset__.num_static_fields_},
    {"art::mirror::Class", "num_virtual_methods_", &__Class_offset__.num_virtual_methods_},
    {"art::mirror::Class", "fields_", &__Class_offset__.fields_},
    {"art::mirror::Class", "embedded_vtable_length_", &__Class_offset__.embedded_vtable_length_},
    {"art::mirror::Class", "imt_ptr_", &__Class_offset__.imt_ptr_},
    {"art::mirror::Class", "embedded_vtable_", &__Class_offset__.embedded_vtable_},

    // art::mirror::String
    {"art::mirror::String", nullptr, &__String_size__.THIS},
    {"art::mirror::String", "count_", &__String_offset__.count_},
    {"art::mirror::String", "hash_code_", &__String_offset__.hash_code_},
    {"art::mirror::String", "value_", &__String_offset__.value_},
    {"art::mirror::String", "value_compressed_", &__String_offset__.value_compressed_},

    // art::mirror::Array
    {"art::mirror::Array", "length_", &__Array_offset__.length_},
    {"art::mirror::Array", "first_element_", &__Array_offset__.first_element_},

    // art::mirror::DexCache
    {"art::mirror::DexCache", "class_loader_", &__DexCache_offset__.class_loader_},
    {"art::mirror::DexCache", "location_", &__DexCache_offset__.location_},
    {"art::mirror::DexCache", "dex_file_", &__DexCache_offset__.dex_file_},
    {"art::mirror::DexCache", "preresolved_strings_", &__DexCache_offset__.preresolved_strings_},
    {"art::mirror::DexCache", "resolved_call_sites_", &__DexCache_offset__.resolved_call_sites_},
    {"art::mirror::DexCache", "resolved_fields_", &__DexCache_offset__.resolved_fields_},
    {"art::mirror::DexCache", "resolved_method_types_", &__DexCache_offset__.resolved_method_types_},
    {"art::mirror::DexCache", "resolved_methods_", &__DexCache_offset__.resolved_methods_},
    {"art::mirror::DexCache", "resolved_types_", &__DexCache_offset__.resolved_types_},
    {"art::mirror::DexCache", "strings_", &__DexCache_offset__.strings_},
    {"art::mirror::DexCache", "num_preresolved_strings_", &__DexCache_offset__.num_preresolved_strings_},
    {"art::mirror::DexCache", "num_resolved_call_sites_", &__DexCache_offset__.num_resolved_call_sites_},
    {"art::mirror::DexCache", "num_resolved_fields_", &__DexCache_offset__.num_resolved_fields_},
    {"art::mirror::DexCache", "num_resolved_method_types_", &__DexCache_offset__.num_resolved_method_types_},
    {"art::mirror::DexCache", "num_resolved_methods_", &__DexCache_offset__.num_resolved_methods_},
    {"art::mirror::DexCache", "num_resolved_types_", &__DexCache_offset__.num_resolved_types_},
    {"art::mirror::DexCache", "num_strings_", &__DexCache_offset__.num_strings_},
    {"art::mirror::DexCache", "resolved_fields_array_", &__DexCache_offset__.resolved_fields_array_},
    {"art::mirror::DexCache", "resolved_method_types_array_", &__DexCache_offset__.resolved_method_types_array_},
    {"art::mirror::DexCache", "resolved_methods_array_", &__DexCache_offset__.resolved_methods_array_},
    {"art::mirror::DexCache", "resolved_types_array_", &__DexCache_offset__.resolved_types_array_},
    {"art::mirror::DexCache", "strings_array_", &__DexCache_offset__.strings_array_},

    // art::StandardDexFile::CodeItem
    {"art::StandardDexFile::CodeItem", "registers_size_", &__StandardDexFile_CodeItem_offset__.registers_size_},
    {"art::StandardDexFile::CodeItem", "ins_size_", &__StandardDexFile_CodeItem_offset__.ins_size_},
    {"art::StandardDexFile::CodeItem", "outs_size_", &__StandardDexFile_CodeItem_offset__.outs_size_},
    {"art::StandardDexFile::CodeItem", "tries_size_", &__StandardDexFile_CodeItem_offset__.tries_size_},
    {"art::StandardDexFile::CodeItem", "debug_info_off_", &__StandardDexFile_CodeItem_offset__.debug_info_off_},
    {"art::StandardDexFile::CodeItem", "insns_size_in_code_units_", &__StandardDexFile_CodeItem_offset__.insns_size_in_code_units_},
    {"art::StandardDexFile::CodeItem", "insns_", &__StandardDexFile_CodeItem_offset__.insns_},

    // art::CompactDexFile::CodeItem
    {"art::CompactDexFile::CodeItem", "fields_", &__CompactDexFile_CodeItem_offset__.fields_},
    {"art::CompactDexFile::CodeItem", "insns_count_and_flags_", &__CompactDexFile_CodeItem_offset__.insns_count_and_flags_},
    {"art::CompactDexFile::CodeItem", "insns_", &__CompactDexFile_CodeItem_offset__.insns_},

    // art::DexFile
    {"art::DexFile", "begin_", &__DexFile_offset__.begin_},
    {"art::DexFile", "size_", &__DexFile_offset__.size_},
    {"art::DexFile", "data_begin_", &__DexFile_offset__.data_begin_},
    {"art::DexFile", "data_size_", &__DexFile_offset__.data_size_},
    {"art::DexFile", "location_", &__DexFile_offset__.location_},
    {"art::DexFile", "location_checksum_", &__DexFile_offset__.location_checksum_},
    {"art::DexFile", "header_", &__DexFile_offset__.header_},
    {"art::DexFile", "string_ids_", &__DexFile_offset__.string_ids_},
    {"art::DexFile", "type_ids_", &__DexFile_offset__.type_ids_},
    {"art::DexFile", "field_ids_", &__DexFile_offset__.field_ids_},
    {"art::DexFile", "method_ids_", &__DexFile_offset__.method_ids_},
    {"art::DexFile", "proto_ids_", &__DexFile_offset__.proto_ids_},
    {"art::DexFile", "class_defs_", &__DexFile_offset__.class_defs_},
    {"art::DexFile", "method_handles_", &__DexFile_offset__.method_handles_},
    {"art::DexFile", "num_method_handles_", &__DexFile_offset__.num_method_handles_},
    {"art::DexFile", "call_site_ids_", &__DexFile_offset__.call_site_ids_},
    {"art::DexFile", "num_call_site_ids_", &__DexFile_offset__.num_call_site_ids_},
    {"art::DexFile", "hiddenapi_class_data_", &__DexFile_offset__.hiddenapi_class_data_},
    {"art::DexFile", "oat_dex_file_", &__DexFile_offset__.oat_dex_file_},
    {"art::DexFile", "container_", &__DexFile_offset__.container_},
    {"art::DexFile", "is_compact_dex_", &__DexFile_offset__.is_compact_dex_},

    // art::ArtMethod
    {"art::ArtMethod", nullptr, &__ArtMethod_size__.THIS},
    {"art::ArtMethod", "declaring_class_", &__ArtMethod_offset__.declaring_class_},
    {"art::ArtMethod", "access_flags_", &__ArtMethod_offset__.access_flags_},
    {"art::ArtMethod", "dex_code_item_offset_", &__ArtMethod_offset__.dex_code_item_offset_},
    {"art::ArtMethod", "dex_method_index_", &__ArtMethod_offset__.dex_method_index_},
    {"art::ArtMethod", "method_index_", &__ArtMethod_offset__.method_index_},
    {"art::ArtMethod", "hotness_count_", &__ArtMethod_offset__.hotness_count_},
    {"art::ArtMethod", "imt_index_", &__ArtMethod_offset__.imt_index_},
    {"art::ArtMethod", "ptr_sized_fields_", &__ArtMethod_offset__.ptr_sized_fields_},

    // art::ArtMethod::PtrSizedFields
    {"art::ArtMethod::PtrSizedFields", "dex_cache_resolved_methods_", &__PtrSizedFields_offset__.dex_cache_resolved_methods_},
    {"art::ArtMethod::PtrSizedFields", "data_", &__PtrSizedFields_offset__.data_},
    {"art::ArtMethod::PtrSizedFields", "entry_point_from_quick_compiled_code_", &__PtrSizedFields_offset__.entry_point_from_quick_compiled_code_},

    // art::ArtField
    {"art::ArtField", nullptr, &__ArtField_size__.THIS},
    {"art::ArtField", "declaring_class_", &__ArtField_offset__.declaring_class_},
    {"art::ArtField", "access_flags_", &__ArtField_offset__.access_flags_},
    {"art::ArtField", "field_dex_idx_", &__ArtField_offset__.field_dex_idx_},
    {"art::ArtField", "offset_", &__ArtField_offset__.offset_},

    // art::OatFile
    {"art::OatFile", "vdex_", &__OatFile_offset__.vdex_},
    {"art::OatFile", "begin_", &__OatFile_offset__.begin_},
    {"art::OatFile", "is_executable_", &__OatFile_offset__.is_executable_},

    // art::OatDexFile
    {"art::OatDexFile", "oat_file_", &__OatDexFile_offset__.oat_file_},
    {"art::OatDexFile", "oat_class_offsets_pointer_", &__OatDexFile_offset__.oat_class_offsets_pointer_},

    // art::ManagedStack
    {"art::ManagedStack", "tagged_top_quick_frame_", &__ManagedStack_offset__.tagged_top_quick_frame_},
    {"art::ManagedStack", "link_", &__ManagedStack_offset__.link_},
    {"art::ManagedStack", "top_shadow_frame_", &__ManagedStack_offset__.top_shadow_frame_},

    // art::ShadowFrame
    {"art::ShadowFrame", "link_", &__ShadowFrame_offset__.link_},
    {"art::ShadowFrame", "method_", &__ShadowFrame_offset__.method_},
    {"art::ShadowFrame", "result_register_", &__ShadowFrame_offset__.result_register_},
    {"art::ShadowFrame", "dex_pc_ptr_", &__ShadowFrame_offset__.dex_pc_ptr_},
    {"art::ShadowFrame", "dex_instructions_", &__ShadowFrame_offset__.dex_instructions_},
    {"art::ShadowFrame", "lock_count_data_", &__ShadowFrame_offset__.lock_count_data_},
    {"art::ShadowFrame", "number_of_vregs_", &__ShadowFrame_offset__.number_of_vregs_},
    {"art::ShadowFrame", "dex_pc_", &__ShadowFrame_offset__.dex_pc_},
    {"art::ShadowFrame", "cached_hotness_countdown_", &__ShadowFrame_offset__.cached_hotness_countdown_},
    {"art::ShadowFrame", "hotness_countdown_", &__ShadowFrame_offset__.hotness_countdown_},
    {"art::ShadowFrame", "frame_flags_", &__ShadowFrame_offset__.frame_flags_},
    {"art::ShadowFrame", "vregs_", &__ShadowFrame_offset__.vregs_},

    // art::Monitor
    {"art::Monitor", "monitor_lock_", &__Monitor_offset__.monitor_lock_},
    {"art::Monitor", "owner_", &__Monitor_offset__.owner_},
    {"art::Monitor", "obj_", &__Monitor_offset__.obj_},

    // art::gc::space::Space
    {"art::gc::space::Space", "name_", &__Space_offset__.name_},

    // art::gc::space::ContinuousSpace
    {"art::gc::space::ContinuousSpace", "begin_", &__ContinuousSpace_offset__.begin_},
    {"art::gc::space::ContinuousSpace", "end_", &__ContinuousSpace_offset__.end_},
    {"art::gc::space::ContinuousSpace", "limit_", &__ContinuousSpace_offset__.limit_},

    // art::gc::space::RegionSpace
    {"art::gc::space::RegionSpace", "num_regions_", &__RegionSpace_offset__.num_regions_},
    {"art::gc::space::RegionSpace", "regions_", &__RegionSpace_offset__.regions_},
    {"art::gc::space::RegionSpace", "mark_bitmap_", &__RegionSpace_offset__.mark_bitmap_},

    // art::gc::space::RegionSpace::Region
    {"art::gc::space::RegionSpace::Region", nullptr, &__Region_size__.THIS},
    {"art::gc::space::RegionSpace::Region", "idx_", &__Region_offset__.idx_},
    {"art::gc::space::RegionSpace::Region", "live_bytes_", &__Region_offset__.live_bytes_},
    {"art::gc::space::RegionSpace::Region", "begin_", &__Region_offset__.begin_},
    {"art::gc::space::RegionSpace::Region", "thread_", &__Region_offset__.thread_},
    {"art::gc::space::RegionSpace::Region", "top_", &__Region_offset__.top_},
    {"art::gc::space::RegionSpace::Region", "end_", &__Region_offset__.end_},
    {"art::gc::space::RegionSpace::Region", "objects_allocated_", &__Region_offset__.objects_allocated_},
    {"art::gc::space::RegionSpace::Region", "alloc_time_", &__Region_offset__.alloc_time_},
    {"art::gc::space::RegionSpace::Region", "is_newly_allocated_", &__Region_offset__.is_newly_allocated_},
    {"art::gc::space::RegionSpace::Region", "is_a_tlab_", &__Region_offset__.is_a_tlab_},
    {"art::gc::space::RegionSpace::Region", "state_", &__Region_offset__.state_},
    {"art::gc::space::RegionSpace::Region", "type_", &__Region_offset__.type_},

    // art::gc::space::LargeObjectSpace
    {"art::gc::space::LargeObjectSpace", "lock_", &__LargeObjectSpace_offset__.lock_},
    {"art::gc::space::LargeObjectSpace", "num_bytes_allocated_", &__LargeObjectSpace_offset__.num_bytes_allocated_},
    {"art::gc::space::LargeObjectSpace", "num_objects_allocated_", &__LargeObjectSpace_offset__.num_objects_allocated_},
    {"art::gc::space::LargeObjectSpace", "total_bytes_allocated_", &__LargeObjectSpace_offset__.total_bytes_allocated_},
    {"art::gc::space::LargeObjectSpace", "total_objects_allocated_", &__LargeObjectSpace_offset__.total_objects_allocated_},
    {"art::gc::space::LargeObjectSpace", "begin_", &__LargeObjectSpace_offset__.begin_},
    {"art::gc::space::LargeObjectSpace", "end_", &__LargeObjectSpace_offset__.end_},

    // art::gc::space::LargeObjectMapSpace
    {"art::gc::space::LargeObjectMapSpace", "large_objects_", &__LargeObjectMapSpace_offset__.large_objects_},

    // art::gc::space::LargeObjectMapSpace::LargeObject
    {"art::gc::space::LargeObjectMapSpace::LargeObject", "mem_map", &__LargeObject_offset__.mem_map},
    {"art::gc::space::LargeObjectMapSpace::LargeObject", "is_zygote", &__LargeObject_offset__.is_zygote},

    // art::gc::space::FreeListSpace::AllocationInfo
    {"art::gc::space::FreeListSpace::AllocationInfo", nullptr, &__AllocationInfo_size__.THIS},
    {"art::gc::space::FreeListSpace::AllocationInfo", "prev_free_", &__AllocationInfo_offset__.prev_free_},
    {"art::gc::space::FreeListSpace::AllocationInfo", "alloc_size_", &__AllocationInfo_offset__.alloc_size_},

    // art::gc::space::FreeListSpace
    {"art::gc::space::FreeListSpace", "mem_map_", &__FreeListSpace_offset__.mem_map_},
    {"art::gc::space::FreeListSpace", "allocation_info_map_", &__FreeListSpace_offset__.allocation_info_map_},
    {"art::gc::space::FreeListSpace", "allocation_info_", &__FreeListSpace_offset__.allocation_info_},
    {"art::gc::space::FreeListSpace", "free_end_", &__FreeListSpace_offset__.free_end_},
    {"art::gc::space::FreeListSpace", "free_blocks_", &__FreeListSpace_offset__.free_blocks_},

    // art::gc::space::ContinuousMemMapAllocSpace
    {"art::gc::space::ContinuousMemMapAllocSpace", "live_bitmap_", &__ContinuousMemMapAllocSpace_offset__.live_bitmap_},
    {"art::gc::space::ContinuousMemMapAllocSpace", "mark_bitmap_", &__ContinuousMemMapAllocSpace_offset__.mark_bitmap_},
    {"art::gc::space::ContinuousMemMapAllocSpace", "temp_bitmap_", &__ContinuousMemMapAllocSpace_offset__.temp_bitmap_},

    // art::gc::space::BumpPointerSpace
    {"art::gc::space::BumpPointerSpace", "main_block_size_", &__BumpPointerSpace_offset__.main_block_size_},
    {"art::gc::space::BumpPointerSpace", "num_blocks_", &__BumpPointerSpace_offset__.num_blocks_},
    {"art::gc::space::BumpPointerSpace", "block_sizes_", &__BumpPointerSpace_offset__.block_sizes_},
    {"art::gc::space::BumpPointerSpace", "black_dense_region_size_", &__BumpPointerSpace_offset__.black_dense_region_size_},

    // art::gc::accounting::ContinuousSpaceBitmap
    {"art::gc::accounting::ContinuousSpaceBitmap", "mem_map_", &__ContinuousSpaceBitmap_offset__.mem_map_},
    {"art::gc::accounting::ContinuousSpaceBitmap", "bitmap_begin_", &__ContinuousSpaceBitmap_offset__.bitmap_begin_},
    {"art::gc::accounting::ContinuousSpaceBitmap", "bitmap_size_", &__ContinuousSpaceBitmap_offset__.bitmap_size_},
    {"art::gc::accounting::ContinuousSpaceBitmap", "heap_begin_", &__ContinuousSpaceBitmap_offset__.heap_begin_},

    // art::jit::Jit
    {"art::jit::Jit", "code_cache_", &__Jit_offset__.code_cache_},

    // art::jit::JitCodeCache
    {"art::jit::JitCodeCache", "code_map_", &__JitCodeCache_offset__.code_map_},
    {"art::jit::JitCodeCache", "exec_pages_", &__JitCodeCache_offset__.exec_pages_},
    {"art::jit::JitCodeCache", "shared_region_", &__JitCodeCache_offset__.shared_region_},
    {"art::jit::JitCodeCache", "private_region_", &__JitCodeCache_offset__.private_region_},
    {"art::jit::JitCodeCache", "jni_stubs_map_", &__JitCodeCache_offset__.jni_stubs_map_},
    {"art::jit::JitCodeCache", "method_code_map_", &__JitCodeCache_offset__.method_code_map_},
    {"art::jit::JitCodeCache", "zygote_exec_pages_", &__JitCodeCache_offset__.zygote_exec_pages_},
    {"art::jit::JitCodeCache", "zygote_map_", &__JitCodeCache_offset__.zygote_map_},

    // art::jit::JitMemoryRegion
    {"art::jit::JitMemoryRegion", "exec_pages_", &__JitMemoryRegion_offset__.exec_pages_},

    // art::JavaVMExt
    {"art::JavaVMExt", "globals_", &__JavaVMExt_offset__.globals_},
    {"art::JavaVMExt", "weak_globals_", &__JavaVMExt_offset__.weak_globals_},

    // art::JNIEnvExt
    {"art::JNIEnvExt", "functions", &__JNIEnvExt_offset__.functions},
    {"art::JNIEnvExt", "self_", &__JNIEnvExt_offset__.self_},
    {"art::JNIEnvExt", "vm_", &__JNIEnvExt_offset__.vm_},
    {"art::JNIEnvExt", "locals_", &__JNIEnvExt_offset__.locals_},

    // art::IrtEntry
    {"art::IrtEntry", nullptr, &__IrtEntry_size__.THIS},
    {"art::IrtEntry", "serial_", &__IrtEntry_offset__.serial_},
    {"art::IrtEntry", "references_", &__IrtEntry_offset__.references_},

    // art::IndirectReferenceTable
    {"art::IndirectReferenceTable", "segment_state_", &__IndirectReferenceTable_offset__.segment_state_},
    {"art::IndirectReferenceTable", "table_mem_map_", &__IndirectReferenceTable_offset__.table_mem_map_},
    {"art::IndirectReferenceTable", "table_", &__IndirectReferenceTable_offset__.table_},
    {"art::IndirectReferenceTable", "kind_", &__IndirectReferenceTable_offset__.kind_},
    {"art::IndirectReferenceTable", "top_index_", &__IndirectReferenceTable_offset__.top_index_},

    // art::jni::LocalReferenceTable
    {"art::jni::LocalReferenceTable", "segment_state_", &__LocalReferenceTable_offset__.segment_state_},
    {"art::jni::LocalReferenceTable", "tables_", &__LocalReferenceTable_offset__.tables_},

    // art::jni::LrtEntry
    {"art::jni::LrtEntry", nullptr, &__LrtEntry_size__.THIS},
    {"art::jni::LrtEntry", "root_", &__LrtEntry_offset__.root_},

    // art::ImageHeader
    {"art::ImageHeader", nullptr, &__ImageHeader_size__.THIS},
    {"art::ImageHeader", "image_methods_", &__ImageHeader_offset__.image_methods_},

    // art::BaseMutex
    {"art::BaseMutex", "name_", &__BaseMutex_offset__.name_},

    // art::Mutex
    {"art::Mutex", "exclusive_owner_", &__Mutex_offset__.exclusive_owner_},

    // art::ReaderWriterMutex
    {"art::ReaderWriterMutex", "state_", &__ReaderWriterMutex_offset__.state_},
    {"art::ReaderWriterMutex", "exclusive_owner_", &__ReaderWriterMutex_offset__.exclusive_owner_},

    // art::ConditionVariable
    {"art::ConditionVariable", "name_", &__ConditionVariable_offset__.name_},
    {"art::ConditionVariable", "guard_", &__ConditionVariable_offset__.guard_},
    {"art::ConditionVariable", "sequence_", &__ConditionVariable_offset__.sequence_},
    {"art::ConditionVariable", "num_waiters_", &__ConditionVariable_offset__.num_waiters_},

    // art::MemMap
    {"art::MemMap", "name_", &__MemMap_offset__.name_},
    {"art::MemMap", "begin_", &__MemMap_offset__.begin_},
    {"art::MemMap", "size_", &__MemMap_offset__.size_},

    // art::ZygoteMap
    {"art::jit::ZygoteMap", "array_", &__ZygoteMap_offset__.array_},
    {"art::jit::ZygoteMap", "size_", &__ZygoteMap_offset__.size_},
    {"art::jit::ZygoteMap", "region_", &__ZygoteMap_offset__.region_},
    {"art::jit::ZygoteMap", "compilation_state_", &__ZygoteMap_offset__.compilation_state_},

    // art::OatQuickMethodHeader
    {"art::OatQuickMethodHeader", "code_info_offset_", &__OatQuickMethodHeader_offset__.code_info_offset_},
    {"art::OatQuickMethodHeader", "data_", &__OatQuickMethodHeader_offset__.data_},
    {"art::OatQuickMethodHeader", "vmap_table_offset_", &__OatQuickMethodHeader_offset__.vmap_table_offset_},
    {"art::OatQuickMethodHeader", "method_info_offset_", &__OatQuickMethodHeader_offset__.method_info_offset_},
    {"art::OatQuickMethodHeader", "frame_info_", &__OatQuickMethodHeader_offset__.frame_info_},
    {"art::OatQuickMethodHeader", "code_size_", &__OatQuickMethodHeader_offset__.code_size_},
    {"art::OatQuickMethodHeader", "code_", &__OatQuickMethodHeader_offset__.code_},
    {"art::OatQuickMethodHeader", "mapping_table_offset_", &__OatQuickMethodHeader_offset__.mapping_table_offset_},
    {"art::OatQuickMethodHeader", "gc_map_offset_", &__OatQuickMethodHeader_offset__.gc_map_offset_},

    // art::LengthPrefixedArray
    {"art::LengthPrefixedArray", "size_", &__LengthPrefixedArray_offset__.size_},
    {"art::LengthPrefixedArray", "data_", &__LengthPrefixedArray_offset__.data_},
};

int IniCommand::ApplyDwarfStruct(const dwarf::StructInfo& si) {
    int applied = 0;
    for (size_t i = 0; i < sizeof(kDwarfBindings) / sizeof(kDwarfBindings[0]); ++i) {
        const auto& bind = kDwarfBindings[i];
        if (si.name != bind.dwarf_class)
            continue;

        if (!bind.dwarf_member) {
            // size binding
            if (si.has_size && si.byte_size > 0) {
                uint32_t old = OffsetValue(bind.target);
                auto vit = INSTANCE->values.find(bind.target);
                if (vit != INSTANCE->values.end()) old = vit->second;
                if (old != si.byte_size) {
                    LOGD("  %s size: %u -> %u\n", si.name.c_str(), old, si.byte_size);
                    SetValue(bind.target, si.byte_size);
                    ++applied;
                }
            }
        } else {
            // member offset binding
            for (const auto& mi : si.members) {
                if (!mi.has_offset) continue;
                if (mi.name == bind.dwarf_member) {
                    uint32_t old = OffsetValue(bind.target);
                    auto vit = INSTANCE->values.find(bind.target);
                    if (vit != INSTANCE->values.end()) old = vit->second;
                    if (old != mi.offset) {
                        LOGD("  %s::%s: %u -> %u\n", si.name.c_str(), mi.name.c_str(), old, mi.offset);
                        SetValue(bind.target, mi.offset);
                        ++applied;
                    }
                    break;
                }
            }
        }
    }
    return applied;
}

void IniCommand::ShowIniTable(const char* section, std::unordered_map<std::string, void *>& table) {
    LOGI("[%s]\n", section);
    for (auto& entry : table)
        LOGI("%s=%d\n", entry.first.c_str(), OffsetValue(entry.second));
}

bool IniCommand::SetKeyValue(std::string& key, uint32_t v,
                             std::unordered_map<std::string, void *>& table) {
    auto it = table.find(key);
    if (it != table.end()) {
        if (v != OffsetValue(it->second)) {
            SetValue(it->second, v);
            LOGI("%s=%d\n", it->first.c_str(), v);
            return true;
        }
    }
    return false;
}

uint32_t IniCommand::OffsetValue(void* offset) {
    return *reinterpret_cast<uint32_t *>(offset);
}

void IniCommand::SetValue(void* offset, uint32_t value) {
    INSTANCE->values[offset] = value;
}

void IniCommand::ResetIni() {
    for (auto& value : INSTANCE->values)
        *reinterpret_cast<uint32_t *>(value.first) = value.second;
}

void IniCommand::usage() {
    LOGI("Usage: ini [OPTION] ...\n");
    LOGI("Option:\n");
    LOGI("    -l, --load  <current.ini>      set current ini\n");
    LOGI("    -s, --set   <KEY>=<VALUE>      set entry value\n");
    LOGI("    -c, --clear                    clear current env ini values\n");
}
