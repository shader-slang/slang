// slang-ondemand-ir-stats.h
#ifndef SLANG_ONDEMAND_IR_STATS_H
#define SLANG_ONDEMAND_IR_STATS_H

//
// Temporary instrumentation for the on-demand-IR investigation
// (tmp/on-demand-IR-load/). Not intended to ship.
//
// Answers two questions that decide whether on-demand IR loading is worth
// building, and at what granularity:
//
//   1. How is the fixed cost of `createGlobalSession()` split between AST and
//      IR deserialization, in both time and resident memory?
//   2. Of the IR that is deserialized, how much does a compile actually reach?
//
// Everything here is inert unless SLANG_ONDEMAND_STATS=1 is set in the
// environment, so an instrumented build still measures like a stock one.
//

#include "core/slang-string.h"

#include <cstdint>

namespace Slang
{

namespace OnDemandStats
{

/// True if SLANG_ONDEMAND_STATS is set to something other than 0. Read once.
bool isEnabled();

/// Current resident set size of this process in bytes, or 0 if unavailable.
///
/// Read from /proc/self/statm. RSS is the honest measure here: the fossil blob is
/// mapped rather than copied, so counting heap allocations alone would miss the
/// pages that reading it touches.
uint64_t getCurrentRSSBytes();

/// Records one deserialization phase (AST or IR) for one module.
struct PhaseRecord
{
    const char* phase;     ///< "AST" or "IR"
    String moduleName;     ///< e.g. "core", "glsl"
    double elapsedMs;      ///< wall time of the deserialize call
    int64_t rssDeltaBytes; ///< RSS growth across the call
};

void recordPhase(const PhaseRecord& record);

/// Records the shape of one deserialized IR module: how many instructions it
/// holds and how many bytes of arena those instructions occupy. Together with
/// the serialized size this gives the real in-memory:serialized ratio.
struct IRModuleShape
{
    String moduleName;
    int64_t instCount;
    int64_t globalInstCount;
    int64_t operandSlotCount; ///< entries in operandIndices: type-uses + operands
    int64_t stringByteCount;
    int64_t literalCount;
    int64_t serializedByteCount;    ///< size of the IR RIFF chunk
    int64_t arenaBytesUsed;         ///< IRModule::m_memoryArena after load
    int64_t eagerTierInstCount = 0; ///< insts a symbol-index-only eager tier needs
};

/// Timing/memory breakdown of the three stages inside IR deserialization:
/// copying the flat table out of the mapped blob, allocating one IRInst per
/// entry, and the preorder walk that wires up operands, payloads and children.
struct IRSubPhases
{
    double copyFlatTableMs;
    double allocInstsMs;
    double wireUpMs;
    int64_t copyFlatTableRSSDelta;
    int64_t allocInstsRSSDelta;
};

void recordIRSubPhases(const IRSubPhases& phases);

void recordIRModuleShape(const IRModuleShape& shape);

/// Fills in the fields of the most recently recorded shape that are only known
/// to the caller of the deserializer: which module this was, how large its
/// serialized chunk is, and how much arena the materialized instructions took.
void completeLastIRModuleShape(
    const String& moduleName,
    int64_t serializedByteCount,
    int64_t arenaBytesUsed);

/// Notes that `mangledName` was resolved out of a serialized module by the
/// linker. Counting distinct names gives the fraction of the module a compile
/// actually needs, which is the ceiling on any on-demand scheme.
void recordSymbolUse(const UnownedStringSlice& mangledName, const char* moduleName);

/// Notes that `inst`, a global value belonging to a serialized module, was
/// pulled into the output by the linker. `subtreeInstCount` is the size of the
/// instruction subtree rooted at it.
///
/// This is the figure that matters for sizing a per-symbol on-demand scheme:
/// the sum over pulled symbols approximates how much of a module such a scheme
/// would still have to materialize.
void recordGlobalValueClone(const char* moduleName, const void* inst, int64_t subtreeInstCount);

/// Checks the invariant that per-global-value lazy materialization depends on:
/// that every instruction inside a global value's subtree references only
/// instructions in that same subtree, or module-scope globals.
///
/// If it holds, a global value's body can be decoded independently of every other
/// body, because the only things it can point at are either decoded with it or
/// already present. If it does not, lazy materialization must also pull whatever
/// foreign subtrees are referenced, and the design gets materially harder.
struct CrossBodyReferenceStats
{
    int64_t bodyInstsExamined = 0;
    int64_t operandsExamined = 0;
    int64_t refsToOwnSubtree = 0;
    int64_t refsToModuleScopeGlobal = 0;
    int64_t refsToForeignBody = 0; ///< violations: into another global's subtree
    int64_t refsToOtherModule = 0; ///< references leaving this module entirely
};

/// Runs the check above over `irModule` (an `IRModule*`), recording the result.
void analyzeCrossBodyReferences(const char* moduleName, void* irModule);

/// Writes the accumulated report to stderr. Registered to run at process exit
/// when stats are enabled.
void dumpReport();

} // namespace OnDemandStats

} // namespace Slang

#endif
