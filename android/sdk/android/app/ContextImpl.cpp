/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
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
#include "android/app/ContextImpl.h"
#include "android/util/ArrayMap.h"
#include "java/lang/Class.h"
#include "java/lang/String.h"
#include "runtime/mirror/array.h"

namespace android {
namespace app {

java::lang::Object ContextImpl::getSystemService(const char* serviceName) {
    java::lang::Object service = 0x0;
    java::lang::Class registry = java::lang::Class::forName("android.app.SystemServiceRegistry");
    android::util::ArrayMap SYSTEM_SERVICE_FETCHERS = registry.GetStaticObjectField("SYSTEM_SERVICE_FETCHERS");
    for (int i = 0; i < SYSTEM_SERVICE_FETCHERS.size(); ++i) {
        java::lang::String key = SYSTEM_SERVICE_FETCHERS.keyAt(i).Ptr();
        if (key.toString() == serviceName) {
            java::lang::Object fetcher = SYSTEM_SERVICE_FETCHERS.valueAt(i);
            if (fetcher.instanceof("android.app.SystemServiceRegistry$CachedServiceFetcher")) {
                int mCacheIndex = fetcher.GetIntField("mCacheIndex");
                art::mirror::Array mServiceCache = getServiceCache().thiz();
                if (mCacheIndex < mServiceCache.GetLength()) {
                    api::MemoryRef ref(mServiceCache.GetRawData(sizeof(uint32_t), mCacheIndex), mServiceCache);
                    art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), mServiceCache);
                    service = object;
                }
            }
            break;
        }
    }
    return service;
}

} // namespace app
} // namespace android

