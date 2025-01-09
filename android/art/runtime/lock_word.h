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

#ifndef ANDROID_ART_RUNTIME_LOCK_WORD_H_
#define ANDROID_ART_RUNTIME_LOCK_WORD_H_

#include "runtime/monitor.h"
#include "runtime/runtime_globals.h"

namespace art {

class LockWord {
public:
    enum SizeShiftsAndMasks : uint32_t {  // private marker to avoid generate-operator-out.py from processing.
        // Number of bits to encode the state, currently just fat or thin/unlocked or hash code.
        kStateSize = 2,
        kReadBarrierStateSize = 1,
        kMarkBitStateSize = 1,
        // Number of bits to encode the thin lock owner.
        kThinLockOwnerSize = 16,
        // Remaining bits are the recursive lock count. Zero means it is locked exactly once
        // and not recursively.
        kThinLockCountSize = 32 - kThinLockOwnerSize - kStateSize - kReadBarrierStateSize -
            kMarkBitStateSize,

        // Thin lock bits. Owner in lowest bits.
        kThinLockOwnerShift = 0,
        kThinLockOwnerMask = (1 << kThinLockOwnerSize) - 1,
        kThinLockOwnerMaskShifted = kThinLockOwnerMask << kThinLockOwnerShift,
        kThinLockMaxOwner = kThinLockOwnerMask,
        // Count in higher bits.
        kThinLockCountShift = kThinLockOwnerSize + kThinLockOwnerShift,
        kThinLockCountMask = (1 << kThinLockCountSize) - 1,
        kThinLockMaxCount = kThinLockCountMask,
        kThinLockCountOne = 1 << kThinLockCountShift,  // == 65536 (0x10000)
        kThinLockCountMaskShifted = kThinLockCountMask << kThinLockCountShift,

        // State in the highest bits.
        kStateShift = kReadBarrierStateSize + kThinLockCountSize + kThinLockCountShift +
            kMarkBitStateSize,
        kStateMask = (1 << kStateSize) - 1,
        kStateMaskShifted = kStateMask << kStateShift,
        kStateThinOrUnlocked = 0,
        kStateFat = 1,
        kStateHash = 2,
        kStateForwardingAddress = 3,
        kStateForwardingAddressShifted = kStateForwardingAddress << kStateShift,
        kStateForwardingAddressOverflow = (1 + kStateMask - kStateForwardingAddress) << kStateShift,

        // Read barrier bit.
        kReadBarrierStateShift = kThinLockCountSize + kThinLockCountShift,
        kReadBarrierStateMask = (1 << kReadBarrierStateSize) - 1,
        kReadBarrierStateMaskShifted = kReadBarrierStateMask << kReadBarrierStateShift,
        kReadBarrierStateMaskShiftedToggled = ~kReadBarrierStateMaskShifted,

        // Mark bit.
        kMarkBitStateShift = kReadBarrierStateSize + kReadBarrierStateShift,
        kMarkBitStateMask = (1 << kMarkBitStateSize) - 1,
        kMarkBitStateMaskShifted = kMarkBitStateMask << kMarkBitStateShift,
        kMarkBitStateMaskShiftedToggled = ~kMarkBitStateMaskShifted,

        // GC state is mark bit and read barrier state.
        kGCStateSize = kReadBarrierStateSize + kMarkBitStateSize,
        kGCStateShift = kReadBarrierStateShift,
        kGCStateMaskShifted = kReadBarrierStateMaskShifted | kMarkBitStateMaskShifted,
        kGCStateMaskShiftedToggled = ~kGCStateMaskShifted,

        // When the state is kHashCode, the non-state bits hold the hashcode.
        // Note Object.hashCode() has the hash code layout hardcoded.
        kHashShift = 0,
        kHashSize = 32 - kStateSize - kReadBarrierStateSize - kMarkBitStateSize,
        kHashMask = (1 << kHashSize) - 1,
        kMaxHash = kHashMask,

        // Forwarding address shift.
        kForwardingAddressShift = kObjectAlignmentShift,

        kMonitorIdShift = kHashShift,
        kMonitorIdSize = kHashSize,
        kMonitorIdMask = kHashMask,
        kMonitorIdAlignmentShift = 32 - kMonitorIdSize,
        kMonitorIdAlignment = 1 << kMonitorIdAlignmentShift,
        kMaxMonitorId = kMaxHash
    };

    enum LockState {
        kUnlocked,    // No lock owners.
        kThinLocked,  // Single uncontended owner.
        kFatLocked,   // See associated monitor.
        kHashCode,    // Lock word contains an identity hash.
        kForwardingAddress,  // Lock word contains the forwarding address of an object.
    };
    LockWord(uint32_t val) : value_(val) {}
    LockState GetState();
    Monitor FatLockMonitor();
    uint32_t ThinLockOwner();
private:
    uint32_t value_;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_LOCK_WORD_H_
