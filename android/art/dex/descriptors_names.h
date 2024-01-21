/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ANDROID_ART_DEX_DESCRIPTORS_NAMES_H_
#define ANDROID_ART_DEX_DESCRIPTORS_NAMES_H_

#include <string>

namespace art {

void AppendPrettyDescriptor(const char* descriptor, std::string* result, const char* def);
void AppendPrettyDescriptor(const char* descriptor, std::string* result);
std::string DotToDescriptor(const char* class_name);

} // namespace art

#endif  // ANDROID_ART_DEX_DESCRIPTORS_NAMES_H_
