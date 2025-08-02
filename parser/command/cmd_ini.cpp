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
#include "api/core.h"
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
#include <string>

#if defined(__PARSE_INI__)
#include "SimpleIni.h"
#endif // __PARSE_INI__

#define INI_ENTRY(NAME) {#NAME, &NAME}

IniCommand::IniCommand() : Command("ini") {
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
        INI_ENTRY(__LocalReferenceTable_offset__.previous_state_),
        INI_ENTRY(__LocalReferenceTable_offset__.segment_state_),
        INI_ENTRY(__LocalReferenceTable_offset__.small_table_),
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
        INI_ENTRY(__OatFile_offset__.vdex_),
        INI_ENTRY(__OatFile_offset__.begin_),
        INI_ENTRY(__OatFile_offset__.is_executable_),
        INI_ENTRY(__OatDexFile_offset__.oat_file_),
        INI_ENTRY(__OatDexFile_offset__.oat_class_offsets_pointer_),
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
        INI_ENTRY(__Monitor_offset__.monitor_lock_),
        INI_ENTRY(__Monitor_offset__.owner_),
        INI_ENTRY(__Monitor_offset__.obj_),
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
        INI_ENTRY(__PropInfo_size__.THIS),
        INI_ENTRY(__PropArea_size__.THIS),
        INI_ENTRY(__Propbt_size__.THIS),
        INI_ENTRY(__SerializedLogBuffer_size__.THIS),
        INI_ENTRY(__SerializedLogBuffer_size__.vtbl),
        INI_ENTRY(__FdEntry_size__.THIS),
        INI_ENTRY(__FrameData_size__.THIS),
        INI_ENTRY(__ArtField_size__.THIS),
        INI_ENTRY(__LengthPrefixedArray_size__.THIS),
        INI_ENTRY(__MemMap_size__.THIS),
        INI_ENTRY(__ManagedStack_size__.THIS),
        INI_ENTRY(__ShadowFrame_size__.THIS),
        INI_ENTRY(__StandardDexFile_CodeItem_size__.THIS),
        INI_ENTRY(__CompactDexFile_CodeItem_size__.THIS),
        INI_ENTRY(__Object_size__.THIS),
        INI_ENTRY(__Class_size__.THIS),
        INI_ENTRY(__String_size__.THIS),
        INI_ENTRY(__Array_size__.THIS),
        INI_ENTRY(__TypeId_size__.THIS),
        INI_ENTRY(__StringId_size__.THIS),
        INI_ENTRY(__FieldId_size__.THIS),
        INI_ENTRY(__MethodId_size__.THIS),
        INI_ENTRY(__ProtoId_size__.THIS),
        INI_ENTRY(__TypeList_size__.THIS),
        INI_ENTRY(__TypeItem_size__.THIS),
        INI_ENTRY(__HandleScope_size__.THIS),
        INI_ENTRY(__DexFile_size__.THIS),
        INI_ENTRY(__ImageHeader_size__.THIS),
        INI_ENTRY(__DexCache_size__.THIS),
        INI_ENTRY(__Region_size__.THIS),
        INI_ENTRY(__LargeObjectSpace_size__.THIS),
        INI_ENTRY(__LargeObjectMapSpace_size__.THIS),
        INI_ENTRY(__LargeObject_size__.THIS),
        INI_ENTRY(__LargeObjectsPair_size__.THIS),
        INI_ENTRY(__AllocationInfo_size__.THIS),
        INI_ENTRY(__FreeListSpace_size__.THIS),
        INI_ENTRY(__IrtEntry_size__.THIS),
        INI_ENTRY(__LrtEntry_size__.THIS),
        INI_ENTRY(__ArtMethod_size__.THIS),
        INI_ENTRY(__OatQuickMethodHeader_size__.THIS),
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
}

int IniCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    options.load = false;
    options.store = false;
    options.dump_all = true;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"load",    no_argument, 0,  'l'},
        {"set",     no_argument, 0,  's'},
        {0,         0,           0,   0 },
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "ls",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'l':
                options.load = true;
                options.store = false;
                options.dump_all = false;
                break;
            case 's':
                options.load = false;
                options.store = true;
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

#if defined(__PARSE_INI__)
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(argv[options.optind]);
        if (rc < 0) {
            LOGE("Error loading %s file.\n", argv[options.optind]);
            return 0;
        }

        for (auto& section : android_sections) {
            if (ini.GetSectionSize(section.first.c_str()) <= 0)
                continue;

            std::unordered_map<std::string, void *>* android_section = section.second;
            const char* section_name = section.first.c_str();

            for (auto& entry : *android_section) {
                const char* key_name = entry.first.c_str();
                const char* value = ini.GetValue(section_name, key_name, nullptr, nullptr);
                if (value) {
                    uint32_t v = std::atoi(value);
                    if (v != OffsetValue(entry.second)) {
                        SetValue(entry.second, v);
                        LOGI("%s=%d\n", key_name, v);
                    }
                }
            }
        }
#else
        LOGE("Not support parse ini.\n");
#endif
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
        SetKeyValue(key, v, android_offsets);
        SetKeyValue(key, v, android_sizes);
        SetKeyValue(key, v, android_others);
    }
    return 0;
}

void IniCommand::ShowIniTable(const char* section, std::unordered_map<std::string, void *>& table) {
    LOGI("[%s]\n", section);
    for (auto& entry : table)
        LOGI("%s=%d\n", entry.first.c_str(), OffsetValue(entry.second));
}

void IniCommand::SetKeyValue(std::string& key, uint32_t v,
                             std::unordered_map<std::string, void *>& table) {
    auto it = table.find(key);
    if (it != table.end()) {
        if (v != OffsetValue(it->second)) {
            SetValue(it->second, v);
            LOGI("%s=%d\n", it->first.c_str(), v);
        }
    }
}

uint32_t IniCommand::OffsetValue(void* offset) {
    return *reinterpret_cast<uint32_t *>(offset);
}

void IniCommand::SetValue(void* offset, uint32_t value) {
    *reinterpret_cast<uint32_t *>(offset) = value;
}

void IniCommand::usage() {
    LOGI("Usage: ini [OPTION] ...\n");
    LOGI("Option:\n");
    LOGI("    -l, --load <current.ini>       set current ini\n");
    LOGI("    -s, --set  <KEY>=<VALUE>       set entry value\n");
}
