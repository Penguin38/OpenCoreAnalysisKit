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

#ifndef UTILS_BASE_MACROS_H_
#define UTILS_BASE_MACROS_H_

#define LIKELY(X) __builtin_expect(!!(X), 1)
#define UNLIKELY(X) __builtin_expect(!!(X), 0)
#define ALWAYS_INLINE  __attribute__ ((always_inline))
#define NO_INLINE __attribute__ ((noinline))
#define FLATTEN  __attribute__ ((flatten))
#define ALIGNED(x) __attribute__ ((__aligned__(x)))
#define PACKED(x) __attribute__ ((__aligned__(x), __packed__))
#define UNREACHABLE  __builtin_unreachable
#define HIDDEN __attribute__((visibility("hidden")))
#define EXPORT __attribute__((visibility("default")))

#endif // UTILS_BASE_MACROS_H_
