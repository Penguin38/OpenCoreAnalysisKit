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

#ifndef SERVICES_COM_ANDROID_SERVER_AM_ACTIVITYMANAGERSERVICE_H_
#define SERVICES_COM_ANDROID_SERVER_AM_ACTIVITYMANAGERSERVICE_H_

#include "java/lang/Object.h"

namespace com {
namespace android {
namespace server {
namespace am {

class ActivityManagerService : public java::lang::Object {
public:
    ActivityManagerService(uint32_t obj) : java::lang::Object(obj) {}
    ActivityManagerService(java::lang::Object& obj) : java::lang::Object(obj) {}
    ActivityManagerService(art::mirror::Object& obj) : java::lang::Object(obj) {}

    int dump(int argc, char* const argv[]);
    static int Main(int argc, char* const argv[]);
};

} // namespace am
} // namespace server
} // namespace android
} // namespace com

#endif // SERVICES_COM_ANDROID_SERVER_AM_ACTIVITYMANAGERSERVICE_H_
