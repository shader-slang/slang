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
// ranges holds whatever byte intervals reachable code touched.
// isUntracked flags params we declined to analyze (e.g. unbounded
// uniform element type), in which case ranges is empty and the consumer
// should record an untracked marker so per byte queries return
// SLANG_E_NOT_AVAILABLE instead of a misleading false answer.
//
// Two spaces, because two downstream consumers key off different ones
// and they diverge for a constant buffer bound to a non zero descriptor
// set:
//
//   parentSpace        - the space the Uniform parameter category
//                        reports (RegisterSpace offset only). This is
//                        what spIsParameterLocationUsed(Uniform, space)
//                        is queried with, since a loose uniform leaf has
//                        no descriptor set of its own and reports space
//                        zero. The recorded ranges carry this space too.
//   parentBindingSpace - the space of the parent CB or parameter block
//                        binding itself (RegisterSpace offset plus the
//                        ConstantBuffer/DescriptorTableSlot category's
//                        own space). This is what the reflection emitter
//                        matches against via getBindingSpace, so it must
//                        include the descriptor set.
//
// Together with parentBindingIndex, parentBindingSpace identifies the
// parent binding so multiple CBs in a shared register space stay
// disambiguated downstream.
struct ByteGranularityParameterUsageInfo
{
    IRGlobalParam* param;
    UInt parentSpace;
    UInt parentBindingSpace;
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
