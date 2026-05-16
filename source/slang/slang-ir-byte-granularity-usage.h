// slang-ir-byte-granularity-usage.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{

// A contiguous byte range that the IR pass saw being read. Offsets are
// the same ones VariableLayoutReflection.getOffset (category Uniform)
// returns, so callers can match queries against these ranges directly.
struct ByteGranularityUsageRange
{
    UInt offset;
    UInt size;
};

// Per parameter usage record. param is the global shader parameter.
// (parentSpace, parentBindingIndex) identify the param's primary CB
// or parameter block binding so multiple CBs in a shared register
// space stay disambiguated downstream. ranges holds whatever byte
// intervals reachable code touched. isUntracked flags params we
// declined to analyze (e.g. unbounded uniform element type), in which
// case ranges is empty and the consumer should record an untracked
// marker so per byte queries return SLANG_E_NOT_AVAILABLE instead of
// a misleading false answer.
struct ByteGranularityParameterUsageInfo
{
    IRGlobalParam* param;
    UInt parentSpace;
    UInt parentBindingIndex;
    List<ByteGranularityUsageRange> ranges;
    bool isUntracked;
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
