/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "com/android/server/am/ActivityManagerService.h"
#include "android/util/Singleton.h"
#include "java/lang/Class.h"

namespace com {
namespace android {
namespace server {
namespace am {

int ActivityManagerService::dump(int argc, char* const argv[]) {
    LOGI("DUMPSYS OF ACTIVITY (0x%x):\n", Ptr());
    return 0;
}

int ActivityManagerService::Main(int argc, char* const argv[]) {
    java::lang::Class clazz = java::lang::Class::forName("android.app.ActivityManager");
    if (clazz.Ptr()) {
        ::android::util::Singleton IActivityManagerSingleton = clazz.GetStaticObjectField("IActivityManagerSingleton");
        java::lang::Object object = IActivityManagerSingleton.get();
        if (object.instanceof("com.android.server.am.ActivityManagerService")) {
            ActivityManagerService service = object;
            return service.dump(argc, argv);
        }
    }
    return 0;
}

} // namespace am
} // namespace server
} // namespace android
} // namespace com
