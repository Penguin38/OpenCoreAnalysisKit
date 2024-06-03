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

#ifndef ANDROID_ART_RUNTIME_STACK_H_
#define ANDROID_ART_RUNTIME_STACK_H_

#include "runtime/thread.h"
#include "runtime/managed_stack.h"
#include "runtime/art_method.h"
#include <vector>
#include <memory>

/*
 * Our current stack layout.
 * The Dalvik registers come first, followed by the
 * Method*, followed by other special temporaries if any, followed by
 * regular compiler temporary. As of now we only have the Method* as
 * as a special compiler temporary.
 * A compiler temporary can be thought of as a virtual register that
 * does not exist in the dex but holds intermediate values to help
 * optimizations and code generation. A special compiler temporary is
 * one whose location in frame is well known while non-special ones
 * do not have a requirement on location in frame as long as code
 * generator itself knows how to access them.
 *
 *     +-------------------------------+
 *     | IN[ins-1]                     |  {Note: resides in caller's frame}
 *     |       .                       |
 *     | IN[0]                         |
 *     | caller's ArtMethod            |  ... ArtMethod*
 *     +===============================+  {Note: start of callee's frame}
 *     | core callee-save spill        |  {variable sized}
 *     +-------------------------------+
 *     | fp callee-save spill          |
 *     +-------------------------------+
 *     | filler word                   |  {For compatibility, if V[locals-1] used as wide
 *     +-------------------------------+
 *     | V[locals-1]                   |
 *     | V[locals-2]                   |
 *     |      .                        |
 *     |      .                        |  ... (reg == 2)
 *     | V[1]                          |  ... (reg == 1)
 *     | V[0]                          |  ... (reg == 0) <---- "locals_start"
 *     +-------------------------------+
 *     | stack alignment padding       |  {0 to (kStackAlignWords-1) of padding}
 *     +-------------------------------+
 *     | Compiler temp region          |  ... (reg >= max_num_special_temps)
 *     |      .                        |
 *     |      .                        |
 *     | V[max_num_special_temps + 1]  |
 *     | V[max_num_special_temps + 0]  |
 *     +-------------------------------+
 *     | OUT[outs-1]                   |
 *     | OUT[outs-2]                   |
 *     |       .                       |
 *     | OUT[0]                        |
 *     | ArtMethod*                    |  ... (reg == num_total_code_regs == special_temp_value) <<== sp, 16-byte aligned
 *     +===============================+
 */

namespace art {

class StackVisitor {
public:
    // This enum defines a flag to control whether inlined frames are included
    // when walking the stack.
    enum class StackWalkKind {
        kIncludeInlinedFrames,
        kSkipInlinedFrames,
    };

    class JavaFrame {
    public:
        JavaFrame(ArtMethod& m, api::MemoryRef& qf, ShadowFrame& sf)
            : method(m), quick_frame(qf), shadow_frame(sf) {}
        ArtMethod& GetMethod() { return method; }
    private:
        ArtMethod method;
        ShadowFrame shadow_frame = 0x0;
        api::MemoryRef quick_frame = 0x0;
    };

    StackVisitor(Thread* thread, StackWalkKind kind) : thread_(thread), walk_kind_(kind) {}
    std::vector<std::unique_ptr<JavaFrame>>& GetJavaFrames() { return java_frames_; }
    Thread* GetThread() { return thread_; }
    void WalkStack();
    bool VisitFrame();
    ArtMethod GetMethod();
    ~StackVisitor() { java_frames_.clear(); }
private:
    std::vector<std::unique_ptr<JavaFrame>> java_frames_;
    Thread* thread_;
    const StackWalkKind walk_kind_;
    ShadowFrame cur_shadow_frame_ = 0x0;
    api::MemoryRef cur_quick_frame_ = 0x0;
    uint64_t cur_quick_frame_pc_;
};

} // namespace art

#endif  // ANDROID_ART_RUNTIME_STACK_H_
