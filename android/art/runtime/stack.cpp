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
#include "common/exception.h"
#include "runtime/stack.h"
#include "runtime/managed_stack.h"
#include "runtime/art_method.h"

namespace art {

ArtMethod StackVisitor::GetMethod() {
    if (cur_shadow_frame_.Ptr()) {
        return cur_shadow_frame_.GetMethod();
    } else if (cur_quick_frame_.Ptr()) {
        return cur_quick_frame_.valueOf();
    }
    return 0x0;
}

bool StackVisitor::VisitFrame() {
    ArtMethod method = GetMethod();
    std::unique_ptr<JavaFrame> frame = std::make_unique<JavaFrame>(method,
                                                                   cur_quick_frame_,
                                                                   cur_shadow_frame_);
    java_frames_.push_back(std::move(frame));
    return false;
}

void StackVisitor::WalkStack() {
    for (ManagedStack current_fragment = GetThread()->GetTlsPtr().managed_stack();
            current_fragment.Ptr(); current_fragment = current_fragment.link()) {
        try {
            cur_shadow_frame_ = current_fragment.GetTopShadowFrame();
            cur_quick_frame_ = current_fragment.GetTopQuickFrame();
            cur_quick_frame_pc_ = 0;

            if (cur_quick_frame_.Ptr()) {
                ArtMethod method = cur_quick_frame_.valueOf();
                bool header_retrieved = false;
                if (method.IsNative()) {
                    if (current_fragment.GetTopQuickFrameGenericJniTag()) {
                    } else if (current_fragment.GetTopQuickFrameJitJniTag()) {
                    } else {
                    }
                    header_retrieved = true;
                }

                while (method.Ptr()) {
                    if (!header_retrieved) {

                    }
                    header_retrieved = false;

                    if ((walk_kind_ == StackWalkKind::kIncludeInlinedFrames)) {
                        // do nothing
                    }

                    bool should_continue = VisitFrame();
                    if (!should_continue) {
                        return;
                    }

                    method = 0x0;
                }
            } else if (cur_shadow_frame_.Ptr()) {
                do {
                    bool should_continue = VisitFrame();
                    if (!should_continue) {
                        return;
                    }
                    cur_shadow_frame_ = cur_shadow_frame_.link();
                } while(cur_shadow_frame_.Ptr());
            }
        } catch(InvalidAddressException e) {
            LOGI("  ManagedStack* 0x%lx maybe invalid.\n", current_fragment.Ptr());
        }
    }
}

} // namespace art
