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

#ifndef CORE_API_UNWIND_H_
#define CORE_API_UNWIND_H_

#include "api/thread.h"
#include "common/native_frame.h"
#include <vector>
#include <memory>

namespace api {

class UnwindStack {
public:
    UnwindStack(ThreadApi* thread) : thread_(thread) {}
    std::vector<std::unique_ptr<NativeFrame>>& GetNativeFrames() { return native_frames_; }
    ThreadApi* GetThread() { return thread_; }
    virtual ~UnwindStack() { native_frames_.clear(); }
    virtual void WalkStack() = 0;
    virtual void DumpContextRegister(const char* prefix) = 0;
    static std::unique_ptr<UnwindStack> MakeUnwindStack(ThreadApi* thread);
    inline uint64_t GetContextNum() { return uc_num_; }
    void VisitFrame();
protected:
    std::vector<std::unique_ptr<NativeFrame>> native_frames_;
    uint64_t cur_frame_fp_;
    uint64_t cur_frame_sp_;
    uint64_t cur_frame_pc_;
    uint64_t cur_uc_;
    uint64_t cur_num_;
    uint64_t uc_num_;
private:
    ThreadApi* thread_;
};

} // namespace api

#endif // CORE_API_UNWIND_H_
