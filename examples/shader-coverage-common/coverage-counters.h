#pragma once

#include "core/slang-common.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace coverageDemo
{

// Decode the little-endian counter buffer into one uint64_t per counter slot.
// Use the effective CoverageBufferInfo::elementByteWidth (4 or 8), not the requested
// width: a backend may cap it. The caller must provide complete slots after dispatch
// has finished and the bytes are host-visible; no alignment is required.
//
// This only widens values. Reporting still uses hits[entry.counterIndex], since
// multiple metadata entries can share a slot. Keep the original bytes for counters.bin
// so its layout continues to match the manifest's element_stride.
inline std::vector<uint64_t> decodeCoverageCounters(
    const void* rawCounters,
    size_t byteCount,
    uint32_t counterByteWidth)
{
    SLANG_RELEASE_ASSERT(counterByteWidth == 4 || counterByteWidth == 8);
    SLANG_RELEASE_ASSERT(byteCount % counterByteWidth == 0);
    SLANG_RELEASE_ASSERT(rawCounters || byteCount == 0);

    const auto* bytes = static_cast<const uint8_t*>(rawCounters);
    const size_t counterCount = byteCount / counterByteWidth;
    std::vector<uint64_t> hits(counterCount, 0);
    for (size_t counterIndex = 0; counterIndex < counterCount; ++counterIndex)
    {
        const uint8_t* slot = bytes + counterIndex * counterByteWidth;
        for (uint32_t byteIndex = 0; byteIndex < counterByteWidth; ++byteIndex)
            hits[counterIndex] |= uint64_t(slot[byteIndex]) << (byteIndex * 8);
    }
    return hits;
}

} // namespace coverageDemo
