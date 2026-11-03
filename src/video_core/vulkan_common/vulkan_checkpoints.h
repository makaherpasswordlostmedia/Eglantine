// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <atomic>
#include <cstdlib>

#include "common/common_types.h"

namespace Vulkan::Checkpoints {

/// What a VK_NV_device_diagnostic_checkpoints marker stands for (EDEN_NX2_CHECKPOINTS=1): after a
/// device loss, the last markers the GPU reached name the draw or dispatch it hung on.
struct Entry {
    u64 id;
    const char* kind;
    u64 program_a; ///< Vertex program, or the compute program
    u64 program_b; ///< Pixel program
    u32 count;     ///< Vertices or indices, or workgroups
    u32 instances;
};

/// Markers before every dispatch; EDEN_NX2_CHECKPOINTS=all also before every draw (each marker
/// costs the GPU a lot: draws run at a few frames a second with them).
inline bool Enabled() {
    static const bool enabled = std::getenv("EDEN_NX2_CHECKPOINTS") != nullptr;
    return enabled;
}

inline bool DrawsEnabled() {
    static const bool enabled = [] {
        const char* value = std::getenv("EDEN_NX2_CHECKPOINTS");
        return value != nullptr && value[0] == 'a';
    }();
    return enabled;
}

inline std::array<Entry, 1 << 14> ring{};
inline std::atomic<u64> next_id{1};

/// Records an entry; the returned id is the marker to set.
inline u64 Add(const char* kind, u64 program_a, u64 program_b, u32 count, u32 instances) {
    const u64 id = next_id.fetch_add(1);
    ring[id % ring.size()] = Entry{id, kind, program_a, program_b, count, instances};
    return id;
}

inline const Entry* Find(u64 id) {
    const Entry& entry = ring[id % ring.size()];
    return entry.id == id ? &entry : nullptr;
}

} // namespace Vulkan::Checkpoints
