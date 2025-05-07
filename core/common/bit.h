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

#ifndef CORE_COMMON_BIT_H_
#define CORE_COMMON_BIT_H_

#include <type_traits>

#define GENMASK_UL(h, l) (((~0ULL) << (l)) & (~0ULL >> (64 - 1 - (h))))

template<typename T>
constexpr T RoundDown(T x, std::remove_reference_t<T> n) {
  return (x & -n);
}

template<typename T>
constexpr T RoundUp(T x, std::remove_reference_t<T> n) {
  return RoundDown(x + n - 1, n);
}

#endif // CORE_COMMON_BIT_H_
