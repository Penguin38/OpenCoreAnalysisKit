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

#include "dex/descriptors_names.h"
#include <algorithm>

namespace art {

void AppendPrettyDescriptor(const char* descriptor, std::string* result) {
  // Count the number of '['s to get the dimensionality.
  const char* c = descriptor;
  size_t dim = 0;
  while (*c == '[') {
    dim++;
    c++;
  }

  // Reference or primitive?
  bool primitive = false;
  if (*c == 'L') {
    // "[[La/b/C;" -> "a.b.C[][]".
    c++;  // Skip the 'L'.
  } else {
    primitive = true;
    // "[[B" -> "byte[][]".
    switch (*c) {
      case 'B':
        c = "byte";
        break;
      case 'C':
        c = "char";
        break;
      case 'D':
        c = "double";
        break;
      case 'F':
        c = "float";
        break;
      case 'I':
        c = "int";
        break;
      case 'J':
        c = "long";
        break;
      case 'S':
        c = "short";
        break;
      case 'Z':
        c = "boolean";
        break;
      case 'V':
        c = "void";
        break;  // Used when decoding return types.
      default: result->append(descriptor); return;
    }
  }

  // At this point, 'c' is a string of the form "fully/qualified/Type;" or
  // "primitive". In the former case, rewrite the type with '.' instead of '/':
  std::string temp(c);
  if (!primitive) {
    std::replace(temp.begin(), temp.end(), '/', '.');
    // ...and remove the semicolon:
    if (temp.back() == ';') {
      temp.pop_back();
    }
  }
  result->append(temp);

  // Finally, add 'dim' "[]" pairs:
  for (size_t i = 0; i < dim; ++i) {
    result->append("[]");
  }
}

void AppendPrettyDescriptor(const char* descriptor, std::string* result, const char* def) {
  const char* c = descriptor;
  if (*c == '\0') {
    result->append(def);
  } else {
    AppendPrettyDescriptor(descriptor, result);
  }
}

std::string DotToDescriptor(const char* class_name) {
  std::string descriptor(class_name);
  std::replace(descriptor.begin(), descriptor.end(), '.', '/');
  if (descriptor.length() > 0 && descriptor[0] != '[') {
    descriptor = "L" + descriptor + ";";
  }
  return descriptor;
}

} // namespace art
