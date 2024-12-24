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

#include "runtime/image.h"
#include "android.h"

struct ImageHeader_OffsetTable __ImageHeader_offset__;
struct ImageHeader_SizeTable __ImageHeader_size__;

namespace art {

/* 26 ~ 27 kSaveEverythingMethod **/
uint32_t ImageHeader::kNumImageMethodsCount = 7;

void ImageHeader::Init() {
    Android::RegisterSdkListener(Android::M, art::ImageHeader::Init24);
    Android::RegisterSdkListener(Android::N, art::ImageHeader::Init24);
    Android::RegisterSdkListener(Android::O, art::ImageHeader::Init26);
    Android::RegisterSdkListener(Android::P, art::ImageHeader::Init28);
    Android::RegisterSdkListener(Android::Q, art::ImageHeader::Init29);
    Android::RegisterSdkListener(Android::R, art::ImageHeader::Init30);
    Android::RegisterSdkListener(Android::S, art::ImageHeader::Init31);
    Android::RegisterSdkListener(Android::U, art::ImageHeader::Init34);
    Android::RegisterSdkListener(Android::V, art::ImageHeader::Init35);
}

void ImageHeader::Init24() {
    __ImageHeader_offset__ = {
        .image_methods_ = 144,
    };

    __ImageHeader_size__ = {
        .THIS = 216,
    };

    kNumImageMethodsCount = 6;
}

void ImageHeader::Init26() {
    __ImageHeader_offset__ = {
        .image_methods_ = 152,
    };

    __ImageHeader_size__ = {
        .THIS = 216,
    };

    kNumImageMethodsCount = 7;
}

void ImageHeader::Init28() {
    __ImageHeader_offset__ = {
        .image_methods_ = 152,
    };

    __ImageHeader_size__ = {
        .THIS = 232,
    };

    kNumImageMethodsCount = 9;
}

void ImageHeader::Init29() {
    __ImageHeader_offset__ = {
        .image_methods_ = 160,
    };

    __ImageHeader_size__ = {
        .THIS = 248,
    };

    kNumImageMethodsCount = 9;
}

void ImageHeader::Init30() {
    __ImageHeader_offset__ = {
        .image_methods_ = 168,
    };

    __ImageHeader_size__ = {
        .THIS = 256,
    };

    kNumImageMethodsCount = 9;
}

void ImageHeader::Init31() {
    __ImageHeader_offset__ = {
        .image_methods_ = 160,
    };

    __ImageHeader_size__ = {
        .THIS = 248,
    };

    kNumImageMethodsCount = 9;
}

void ImageHeader::Init34() {
    __ImageHeader_offset__ = {
        .image_methods_ = 168,
    };

    __ImageHeader_size__ = {
        .THIS = 256,
    };

    kNumImageMethodsCount = 9;
}

void ImageHeader::Init35() {
    __ImageHeader_offset__ = {
        .image_methods_ = 176,
    };

    __ImageHeader_size__ = {
        .THIS = 264,
    };

    kNumImageMethodsCount = 9;
}

} // namespace art
