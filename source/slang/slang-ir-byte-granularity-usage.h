// slang-ir-byte-granularity-usage.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

struct IRModule;

// A contiguous byte range that the IR pass saw being read. Offsets are
// the same ones VariableLayoutReflection.getOffset (category Uniform)
// returns, so callers can match queries against these ranges directly.
struct ByteGranularityUsageRange
{
    UInt offset;
    UInt size;
};

// Per parameter usage record. ranges holds the byte intervals reachable
// code touched. A param with no record (no parent binding, or an
// unbounded uniform element type) is treated as fully used.
// parentBindingSpace and parentBindingIndex identify the parent CB or
// parameter block, matched by the reflection emitter via getBindingSpace
// so parentBindingSpace includes the descriptor set.
struct ByteGranularityParameterUsageInfo
{
    UInt parentBindingSpace;
    UInt parentBindingIndex;
    List<ByteGranularityUsageRange> ranges;
};

// For each global param wrapped in a constant buffer or parameter block,
// figure out which bytes of its byte addressable storage reachable code
// actually reads. Other byte addressable globals (RT payloads, byte
// address buffer contents, etc.) are skipped because we don't have a
// useful per leaf access model for them. Returns one record per
// qualifying param.
List<ByteGranularityParameterUsageInfo>
collectApproximateByteGranularityUsageInformationForParameterGroups(const IRModule* module);

} // namespace Slang
