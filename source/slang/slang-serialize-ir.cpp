// slang-serialize-ir.cpp
#include "slang-serialize-ir.h"

#include "core/slang-blob-builder.h"
#include "core/slang-common.h"
#include "core/slang-dictionary.h"
#include "core/slang-performance-profiler.h"
#include "core/slang-platform.h"
#include "core/slang-riff.h"
#include "slang-ir-insts-stable-names.h"
#include "slang-ir-insts.h"
#include "slang-ir-validate.h"
#include "slang-serialize-fossil.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"
#include "slang-tag-version.h"
#include "slang.h"

#include <cstdint>
#include <mutex>
#include <thread>

//
#include "slang-serialize-ir.cpp.fiddle"

FIDDLE()
namespace Slang
{

//
// We wrap everything up in an IRModuleInfo, to prepare for the case in which
// we want to serialize some sidecar information to help with on-demand loading
// or backwards compat
//
// For all the aggregate structs here we'll use Fiddle to generate the
// deserialization code.
//
FIDDLE()
struct IRModuleInfo
{
    FIDDLE(...)
    // Include this here so that if we need to change the way we serialize
    // things and maintain backwards compat we can increment this value, for
    // example if we introduce more instructions with weird payloads like
    // IRModuleInst or IRConstants.
    // If we want to support back compat we'll need to change this to a list of
    // accepted values, and branch on that later down.
    const static UInt64 kSupportedSerializationVersion = 1;
    FIDDLE() UInt64 serializationVersion = kSupportedSerializationVersion;
    // Include the specific compiler version in serialized output, in case we
    // ever need to do any version specific workarounds.
    FIDDLE() String fullVersion = SLANG_TAG_VERSION;
    FIDDLE() RefPtr<IRModule> module;
};

//
// All the information necessary to allocate an ordinary instruction, if it's a
// string constant we need to get the length of the string from another list
// later on.
//
FIDDLE()
struct InstAllocInfo
{
    FIDDLE(...)
    FIDDLE() IROp op;
    FIDDLE() uint32_t operandCount;
};

FIDDLE()
struct FlatInstTable
{
    FIDDLE(...)

    // Each IR instruction has:
    //
    // * An opcode
    // * A result type
    // * Zero or more operands
    // * Zero or more children
    //
    // Most instructions are entirely defined by those properties.
    //
    // The instructions that represent simple constants (integers, strings, etc.) are
    // unique in that they have "payload" data that holds their value, instead of having
    // any operands.
    //
    // The deserialization logic doesn't interact with any
    // systems for deduplication or simplification of instructions.

    // All these lists are a flattened representation of these properties of
    // instructions as traversed in preorder.

    // These are the same length, the number of instructions in the module
    // The instAllocInfo list is all that's necessary to allocate an instruction
    FIDDLE() List<InstAllocInfo> instAllocInfo;
    FIDDLE() SerializedArray<Int64> childCounts;
    FIDDLE() List<SourceLoc> sourceLocs;

    // The length of operandIndices is the number of instructions in the module
    // (for typeUse) + the number of operands in the module
    //
    // a nullptr operand is encoded as -1
    FIDDLE() SerializedArray<Int64> operandIndices;

    // The length is equal to the number of strings and blobs in the module
    FIDDLE() SerializedArray<Int64> stringLengths;

    // The length is the sum of all stringLengths, the contents is the
    // concatenation of all their data
    FIDDLE() SerializedArray<uint8_t> stringChars;

    // The length is number of integer/floating constants in the module, and
    // the contents are the bits of those constants
    FIDDLE() SerializedArray<UInt64> literals;
};

// For debugging
[[maybe_unused]] static void dumpFlatInstTableStats(
    const FlatInstTable& table,
    const char* label = nullptr)
{
    if (label)
    {
        fprintf(stderr, "=== FlatInstTable Stats: %s ===\n", label);
    }
    else
    {
        fprintf(stderr, "=== FlatInstTable Stats ===\n");
    }

    // Basic instruction counts
    auto instCount = table.instAllocInfo.getCount();
    fprintf(stderr, "Instruction count: %zu\n", (size_t)instCount);

    // Verify consistency
    if (table.childCounts.getCount() != instCount)
    {
        fprintf(
            stderr,
            "WARNING: childCounts size (%zu) != instruction count (%zu)\n",
            (size_t)table.childCounts.getCount(),
            (size_t)instCount);
    }
    if (table.sourceLocs.getCount() != instCount)
    {
        fprintf(
            stderr,
            "WARNING: sourceLocs size (%zu) != instruction count (%zu)\n",
            (size_t)table.sourceLocs.getCount(),
            (size_t)instCount);
    }

    // Count string/blob instructions
    Int64 stringBlobInstCount = 0;
    for (const auto& allocInfo : table.instAllocInfo)
    {
        const IROp op = allocInfo.op;
        if (op == kIROp_StringLit || op == kIROp_BlobLit)
        {
            stringBlobInstCount++;
        }
    }

    fprintf(stderr, "String/blob instruction count: %zu\n", (size_t)stringBlobInstCount);
    fprintf(stderr, "stringLengths array size: %zu\n", (size_t)table.stringLengths.getCount());

    // Verify string/blob consistency
    if (stringBlobInstCount != table.stringLengths.getCount())
    {
        fprintf(
            stderr,
            "ERROR: String/blob instruction count (%zu) != stringLengths size (%zu)\n",
            (size_t)stringBlobInstCount,
            (size_t)table.stringLengths.getCount());
    }

    // Verify string data consistency
    Int64 expectedStringDataSize = 0;
    for (auto len : table.stringLengths)
    {
        expectedStringDataSize += len;
    }

    fprintf(stderr, "Expected string data size: %zu bytes\n", (size_t)expectedStringDataSize);
    fprintf(stderr, "Actual stringChars size: %zu bytes\n", (size_t)table.stringChars.getCount());

    if (expectedStringDataSize != table.stringChars.getCount())
    {
        fprintf(
            stderr,
            "ERROR: Expected string data size (%zu) != actual stringChars size (%zu)\n",
            (size_t)expectedStringDataSize,
            (size_t)table.stringChars.getCount());
    }

    // Operand statistics
    auto operandCount = table.operandIndices.getCount() - instCount;
    fprintf(stderr, "Total operands: %zu\n", (size_t)operandCount);
    if (instCount > 0)
    {
        fprintf(
            stderr,
            "Average operands per instruction: %.2f\n",
            (double)operandCount / instCount);
    }

    // Count null operands
    Int64 nullOperandCount = 0;
    for (auto idx : table.operandIndices)
    {
        if (idx == -1)
            nullOperandCount++;
    }
    fprintf(
        stderr,
        "Null operands: %zu (%.1f%%)\n",
        (size_t)nullOperandCount,
        table.operandIndices.getCount() > 0
            ? 100.0 * nullOperandCount / table.operandIndices.getCount()
            : 0.0);

    // String/blob statistics
    if (table.stringLengths.getCount() > 0)
    {
        Int64 maxLength = 0;
        for (auto len : table.stringLengths)
        {
            if (len > maxLength)
                maxLength = len;
        }
        fprintf(
            stderr,
            "Average string length: %.1f bytes\n",
            (double)expectedStringDataSize / table.stringLengths.getCount());
        fprintf(stderr, "Max string length: %zu bytes\n", (size_t)maxLength);
    }

    // Literal constants
    fprintf(stderr, "Literal constants: %zu\n", (size_t)table.literals.getCount());

    // Memory usage estimation
    size_t totalMemory = 0;
    totalMemory += table.instAllocInfo.getCount() * sizeof(InstAllocInfo);
    totalMemory += table.childCounts.getCount() * sizeof(Int64);
    totalMemory += table.sourceLocs.getCount() * sizeof(SourceLoc);
    totalMemory += table.operandIndices.getCount() * sizeof(Int64);
    totalMemory += table.stringLengths.getCount() * sizeof(Int64);
    totalMemory += table.stringChars.getCount() * sizeof(uint8_t);
    totalMemory += table.literals.getCount() * sizeof(UInt64);

    fprintf(
        stderr,
        "Estimated memory usage: %zu bytes (%.2f MB)\n",
        totalMemory,
        totalMemory / (1024.0 * 1024.0));

    fprintf(stderr, "===========================\n");
}


//
// We need some small amount of additional context to serialize IR Modules, keep track of that here
//
struct IRSerialReadContext;
struct IRSerialWriteContext;

// Specialize to the reader/writer for the specific backend we're targeting
// instead of ISerializerImpl to avoid some virtual function calls
using IRWriteSerializer = Serializer<Fossil::SerialWriter, IRSerialWriteContext>;
using IRReadSerializer = Serializer<Fossil::SerialReader, IRSerialReadContext>;

struct IRSerialWriteContext : SourceLocSerialContext
{
    IRSerialWriteContext(SerialSourceLocWriter* sourceLocWriter)
        : _sourceLocWriter(sourceLocWriter)
    {
    }

    virtual void handleIRModule(IRWriteSerializer const& serializer, IRModule*& value);
    virtual void handleName(IRWriteSerializer const& serializer, Name*& value);
    virtual SerialSourceLocWriter* getSourceLocWriter() override { return _sourceLocWriter; }

    SerialSourceLocWriter* _sourceLocWriter;
};

struct IRSerialReadContext : SourceLocSerialContext, RefObject
{
    /// `blobHoldingSerializedData` may be null, and is retained when it is not.
    ///
    /// Deferred instruction bodies are decoded long after this read returns, out of
    /// spans that point into the serialized bytes rather than copies of them. Whoever
    /// owns those bytes therefore has to outlive the `IRModule`. Retaining the blob
    /// here makes that ownership explicit and local; when the caller has no blob to
    /// give (the bytes are a caller-local buffer), bodies are not deferred at all.
    IRSerialReadContext(
        Session* session,
        SerialSourceLocReader* sourceLocReader,
        ISlangBlob* blobHoldingSerializedData)
        : _session(session)
        , _sourceLocReader(sourceLocReader)
        , _blobHoldingSerializedData(blobHoldingSerializedData)
    {
    }

    ISlangBlob* getBlobHoldingSerializedData() const { return _blobHoldingSerializedData; }

    virtual void handleIRModule(IRReadSerializer const& serializer, IRModule*& value);
    virtual void handleName(IRReadSerializer const& serializer, Name*& value);
    virtual SerialSourceLocReader* getSourceLocReader() override { return _sourceLocReader; }

    // Used to allocate an IRModule
    Session* _session;

    //
    SerialSourceLocReader* _sourceLocReader;

    // The blob the serialized bytes live in, or null when the caller read them from
    // storage it owns itself. Retained, because deferred instruction bodies are decoded
    // out of spans that point into these bytes long after the read returns.
    ComPtr<ISlangBlob> _blobHoldingSerializedData;

    // The module in which we will allocate our instructions
    RefPtr<IRModule> _module;

    //
    bool _foundUnrecognizedInstructions = false;
};

SLANG_DECLARE_FOSSILIZED_AS(Name, String);

/// Fossilized representation of a `IRModule`
struct Fossilized_IRModule;

SLANG_DECLARE_FOSSILIZED_TYPE(IRModule, Fossilized_IRModule);

// IROps are serialized as integers, and given a stable name
SLANG_DECLARE_FOSSILIZED_AS(IROp, FossilUInt);

template<typename S>
void serialize(S const& serializer, IROp& value)
{
    auto stableName = isWriting(serializer) ? getOpcodeStableName(value) : kInvalidStableName;
    serializeEnum(serializer, stableName);
    // if we're reading
    if constexpr (std::is_same_v<S, IRReadSerializer>)
    {
        value = getStableNameOpcode(stableName);
        // It's possible we're reading a module serialized by a future version of
        // Slang with as-yet unknown instructions.
        // if this is the case, return IRUnrecognized and we can handle it later
        if (value == kIROp_Invalid)
        {
            value = kIROp_Unrecognized;
            serializer.getContext()->_foundUnrecognizedInstructions = true;
        }
    }
}

//
// Serialize Names via the name pool on the session, this is used just for the
// IRModule name member.
//
template<typename S>
void serializeObject(S const& serializer, Name*& value, Name*)
{
    serializer.getContext()->handleName(serializer, value);
}

void IRSerialWriteContext::handleName(IRWriteSerializer const& serializer, Name*& value)
{
    serialize(serializer, value->text);
}

void IRSerialReadContext::handleName(IRReadSerializer const& serializer, Name*& value)
{
    String text;
    serialize(serializer, text);
    value = _session->getNamePool()->getName(text);
}

//
// This splice handles any aggregate types, a similar splice is well documented
// in slang-serialize-ast.cpp
//
#if 0 // FIDDLE TEMPLATE:
% irStructTypes = {
%   Slang.IRModuleInfo,
%   Slang.FlatInstTable,
%   Slang.InstAllocInfo,
% }
%
% for _,T in ipairs(irStructTypes) do

/// Fossilized representation of a `$T`
struct Fossilized_$T;

SLANG_DECLARE_FOSSILIZED_TYPE($T, Fossilized_$T);

/// Serialize a `$T`
template<typename S>
void serialize(S const& serializer, $T& value);
%end
%for _,T in ipairs(irStructTypes) do
/// Fossilized representation of a value of type `$T`
struct Fossilized_$T
%   if T.directSuperClass then
    : public Fossilized<$(T.directSuperClass)>
%   else
    : public FossilizedRecordVal
%   end
{
%   for i,f in ipairs(T.directFields) do
    Fossilized<decltype($T::$f)> $f;
    const static Index $(f)_fieldIndex = $(i-1);
%   end
};

namespace Fossil{
template<>
struct ValRef<Fossilized_$T> : ValRefBase<Fossilized_$T>
{
public:
    using ValRefBase<Fossilized_$T>::ValRefBase;

%   for i,f in ipairs(T.directFields) do
    AnyValRef get$(tostring(f):gsub("^%l", string.upper))() const
    {
        return as<FossilizedRecordVal>(getAddress(*this))->getField($(i-1));
    }
%   end
};
}
%end

% for _,T in ipairs(irStructTypes) do
/// Serialize a `value` of type `$T`
template<typename S>
void serialize(S const& serializer, $T& value)
{
    SLANG_UNUSED(value);
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
%   if T.directSuperClass then
    serialize(serializer, static_cast<$(T.directSuperClass)&>(value));
%   end
%   for _,f in ipairs(T.directFields) do
    serialize(serializer, value.$f);
%   end
}
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-serialize-ir.cpp.fiddle"
#endif // FIDDLE END

struct Fossilized_IRModule : public FossilizedRecordVal
{
    Fossilized<String> m_name;
    Fossilized<decltype(IRModule::m_version)> m_version;
    Fossilized<FlatInstTable> m_moduleInst;
};

////
//
// After that preamble, this is the interesting stuff now
//
////

//
// Handlers for IRModule, there is a little extra setup to do once top level
// entries are deserialized to set up m_mapMangledNameToGlobalInst, this is
// done at the end of readSerializedModuleIR
//
template<typename S>
void serializeObject(S const& serializer, IRModule*& value, IRModule*)
{
    serializer.getContext()->handleIRModule(serializer, value);
}

static void serializeAsFlatModule(const IRWriteSerializer& serializer, IRModuleInst* moduleInst)
{
    FlatInstTable flat;
    Dictionary<IRInst*, Int64> instMap;
    instMap.add(nullptr, -1);
    List<IRInst*> insts;

    traverseInstsInSerializationOrder(
        moduleInst,
        [&](IRInst* inst)
        {
            const auto thisInstIndex = flat.instAllocInfo.getCount();
            instMap.add(inst, thisInstIndex);
            insts.add(inst);
            flat.instAllocInfo.add(InstAllocInfo{
                .op = inst->m_op,
                .operandCount = inst->operandCount,
            });
            flat.childCounts.add(0);
            flat.sourceLocs.add(inst->sourceLoc);
            inst->scratchData = thisInstIndex; // Store index for child counting

            // Update parent's child count
            if (inst->parent)
            {
                flat.childCounts.mutableAt(inst->parent->scratchData)++;
            }
        });

    for (const auto inst : insts)
    {
        flat.operandIndices.add(instMap.getValue(inst->typeUse.get()));
        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            const auto& operand = inst->getOperand(i);
            flat.operandIndices.add(instMap.getValue(operand));
        }

        if (const auto& c = as<IRConstant>(inst))
        {
            switch (inst->m_op)
            {
            case kIROp_BoolLit:
            case kIROp_IntLit:
                flat.literals.add(bitCast<UInt64>(c->value.intVal));
                break;
            case kIROp_FloatLit:
                flat.literals.add(bitCast<UInt64>(c->value.floatVal));
                break;
            case kIROp_PtrLit:
                // to avoid complaints on 32 bit wasm
                flat.literals.add(UInt64(bitCast<uintptr_t>(c->value.ptrVal)));
                break;
            case kIROp_StringLit:
            case kIROp_BlobLit:
                const auto slice = c->getStringSlice();
                const auto len = slice.getLength();
                flat.stringLengths.add(len);
                flat.stringChars.addRange(reinterpret_cast<const uint8_t*>(slice.begin()), len);
                break;
            }
        }
    }
    // dumpFlatInstTableStats(flat, "serializing");
    serialize(serializer, flat);
}

// ## Depths in the module's preorder walk
//
// The module inst is the root, its globals sit directly under it, and a global's
// decorations and body children sit under those. Three separate pieces of logic depend on
// this model agreeing -- the deferral test in `decodeInst`, the eager-skeleton scan, and
// the depth a replayed body is decoded at -- so the numbers are named rather than written
// out at each site.
static const Int64 kModuleInstDepth = 0;
static const Int64 kGlobalValueDepth = 1;
static const Int64 kBodyChildDepth = 2;

/// Assigns every instruction in `flat`'s preorder table to a deferral region: `-1` for the
/// eager *skeleton* materialized at load time, otherwise an id shared by every instruction
/// in one global value's deferred body.
///
/// The ids are what let `_deferralRegionsAreClosed` tell "reaches into its own body" from
/// "reaches into someone else's", which is the difference between sound and unsound.
///
/// The skeleton is the module inst, its globals, and the run of decorations at the head of
/// each global's child list, including anything nested under those decorations. Everything
/// from a global's first non-decoration child onward is that global's deferrable body.
///
/// This is the single place that cut is decided. The allocation pass reads the result to
/// decide which instructions to allocate at all, and `decodeInst` reads the same result
/// back through `FlatModuleDecoder::isEagerSkeletonInst` to find where a body starts, so
/// the two cannot disagree. They must not: an instruction the allocation pass skips but
/// the decoder does not defer would be wired against an empty slot, and an instruction
/// allocated but deferred anyway would be decoded twice, once by the load walk and once by
/// the replay.
///
/// The rule is deliberately suffix-shaped. A deferred body is recorded as "the last `n`
/// children of this global" and replayed that way, so the deferred set has to be a
/// contiguous tail of the child list. A decoration appearing *after* a body instruction is
/// therefore part of the body rather than an eager exception to it. `IRBuilder` only ever
/// inserts decorations at the head (`addDecoration` calls `insertAtStart`), so the
/// serializer does not emit that shape today -- stating the rule this way means nothing
/// here depends on it continuing not to.
///
/// A decoration is kept eager because the symbol index reads it without materializing.
/// Its children have to be kept for the same reason: they are reachable only through the
/// decoration, and nothing on that path would ever trigger a materialization to supply
/// them, so keeping just the decoration inst would silently give a decoration that has
/// children no children at all.
static void _computeDeferralRegions(
    const FlatInstTable& flat,
    Int64 numInsts,
    List<Int32>& outInstRegion)
{
    outInstRegion.setCount(numInsts);

    // Preorder scan tracking depth, allocating nothing beyond the depth stack.
    // `childCounts` is in the same preorder as the instructions, so a stack of
    // remaining-child counts is enough to recover each instruction's depth.
    List<Int64> remainingChildren;
    Int64 depth = 0;
    // True while the scan is inside one of a global's leading decorations, including
    // anything nested under it.
    bool inEagerDecoration = false;
    // True once the current global's body has started, so that every later child of that
    // global belongs to the body whatever its opcode.
    bool inBody = false;
    // The body currently being scanned, and the next id to hand out. Instructions in the
    // same deferred body share an id; eager instructions get -1.
    Int32 currentRegion = -1;
    Int32 nextRegionId = 0;
    for (Int64 i = 0; i < numInsts; ++i)
    {
        const IROp op = flat.instAllocInfo[i].op;
        const bool isDecoration = op >= kIROp_FirstDecoration && op <= kIROp_LastDecoration;
        // Depth 0 is the module inst and depth 1 its globals; a global's decorations and
        // its body instructions both sit at depth 2. Deeper instructions carry whatever
        // the depth-2 ancestor decided, which is what keeps a decoration's subtree eager
        // and a body's subtree deferred.
        if (depth <= kGlobalValueDepth)
        {
            inEagerDecoration = false;
            inBody = false;
            currentRegion = -1;
        }
        else if (depth == kBodyChildDepth)
        {
            inEagerDecoration = !inBody && isDecoration;
            // The first non-decoration child opens this global's body.
            const bool startsBody = !inEagerDecoration && !inBody;
            inBody = !inEagerDecoration;
            if (startsBody)
                currentRegion = nextRegionId++;
        }
        outInstRegion[i] =
            (depth <= kGlobalValueDepth || inEagerDecoration) ? Int32(-1) : currentRegion;

        remainingChildren.add(flat.childCounts[i]);
        depth++;
        while (remainingChildren.getCount() && remainingChildren.getLast() == 0)
        {
            remainingChildren.removeLast();
            depth--;
        }
        if (remainingChildren.getCount())
            remainingChildren.getLast()--;
    }
}

/// Decoding state for a module's flat instruction table.
///
/// The same walk serves two purposes, which is why it lives in an object rather than a
/// lambda: it runs once over the whole module at load time, and then again over a single
/// subtree each time a deferred body is asked for. Holding the flat table and the
/// instruction array keeps the second use possible -- a body's operands are indices into
/// that array, and may name any module-scope global.
/// True if no instruction reaches into a deferred body other than its own.
///
/// Deferral rests on this. An eager instruction whose operand names an instruction inside
/// a body, or one body naming another, resolves against a slot the load walk deliberately
/// left empty -- `readInstRef` then trips a release assert and takes the whole compile
/// down, far from the cause.
///
/// That invariant was measured over the builtin modules and holds there, but any module
/// read from a retained blob defers, including precompiled user modules produced by the
/// front end. Rather than trust the measurement to generalise, this checks it per load and
/// lets the caller fall back to an eager load -- the same response the blob-containment
/// guard already gives, and a far better failure mode than aborting.
///
/// One linear pass over `operandIndices`, in the order `decodeInst` consumes it: one entry
/// for the type, then one per operand. Allocates nothing.
static bool _deferralRegionsAreClosed(
    const FlatInstTable& flat,
    Int64 numInsts,
    const List<Int32>& instRegion)
{
    const Count operandIndexCount = flat.operandIndices.getCount();
    Int64 cursor = 0;
    for (Int64 i = 0; i < numInsts; ++i)
    {
        const Int64 entryCount = Int64(flat.instAllocInfo[i].operandCount) + 1;
        if (cursor > operandIndexCount - entryCount)
            return false;
        const Int32 region = instRegion[i];
        for (Int64 o = 0; o < entryCount; ++o)
        {
            const auto target = flat.operandIndices[cursor + o];
            if (target == -1)
                continue;
            if (target < 0 || target >= numInsts)
                return false;
            const Int32 targetRegion = instRegion[target];
            // An eager target always resolves. Anything else must be this instruction's
            // own body -- which covers both directions at once, since an eager
            // instruction has `region == -1` and so matches no body.
            if (targetRegion >= 0 && targetRegion != region)
                return false;
        }
        cursor += entryCount;
    }
    return true;
}

struct FlatModuleDecoder : IRDeferredBodyLoader
{
    FlatInstTable flat;
    List<IRInst*> instsList; ///< index -1 is the null slot, hence `insts()`
    IRModule* module = nullptr;

    /// Keeps the serialized bytes alive for as long as bodies can still be decoded.
    ///
    /// The flat table holds spans into this blob rather than copies, so it must not be
    /// released while this decoder can still be asked for a body. Only set when the
    /// caller supplied a blob; deferral is disabled otherwise.
    ComPtr<ISlangBlob> blobHoldingSerializedData;

    /// Set when this decoder allocates an instruction whose opcode this build does not
    /// know. Named apart from `IRSerialReadContext::_foundUnrecognizedInstructions`, which
    /// accumulates the same fact for the caller: the `|=` between them is easy to misread
    /// when both spell it the same way.
    ///
    /// Recorded here rather than on the `IRSerialReadContext`, which this must not
    /// reference: that would close the cycle `IRModule -> decoder -> context -> IRModule`
    /// and leak every module for the life of the process, while a raw pointer would
    /// dangle.
    ///
    /// **Deferral does not hide an unknown opcode**, and it is worth being precise about
    /// why, because the reverse is an easy thing to conclude. Reading the flat table
    /// decodes `instAllocInfo` for *every* instruction, deferred ones included, and
    /// `serialize(S const&, IROp&)` is what turns an unknown stable name into
    /// `kIROp_Unrecognized` -- setting the context's flag as it goes. That happens during
    /// `serialize(serializer, flat)`, before anything decides whether to defer. So both
    /// load paths learn of an unknown opcode at the same point, and both turn it into the
    /// same recoverable read failure.
    ///
    /// Which leaves this flag with nothing to observe: by the time any instruction is
    /// allocated, its op has already been mapped away from `kIROp_Invalid`. It is kept as
    /// a belt-and-braces record for a future reader that allocates from a source other
    /// than the flat table.
    bool sawUnrecognizedOpDuringDecode = false;

    /// Where each deferred body's encoding begins.
    ///
    /// Recorded when the load walk reaches a global value's first non-decoration
    /// child. The payload streams are consumed by running cursors, so replaying a
    /// subtree needs the cursor positions as of its start, not just its index.
    struct DeferredBody
    {
        Int64 firstChildInstIndex;
        Int64 childCount;
        Int64 instCount = 0; ///< instructions in the whole deferred subtree
        Int64 operandCursor;
        Int64 literalCursor;
        Int64 stringLengthCursor;
        Int64 stringDataCursor;
    };
    Dictionary<IRInst*, DeferredBody> deferredBodies;

    /// True while the load walk should defer bodies; forced false during a deferred
    /// decode so nested subtrees materialize fully, and restored on the way out.
    ///
    /// **Not reentrancy-safe on its own.** The save/restore is correct only while no
    /// second decode can interleave: every deferred decode runs under `mutex`, and the
    /// load walk runs before the decoder is reachable by anyone else. A future caller
    /// reaching `decodeInst` from elsewhere must hold that lock or thread the mode
    /// through as a parameter.
    bool deferBodies = false;

    /// The eager-skeleton predicate, borrowed for the duration of the initial load walk.
    ///
    /// Points at the `_computeDeferralRegions` result, which lives on the stack of the
    /// function that runs the walk and is cleared from here when that walk returns. A
    /// later deferred materialization runs with `deferBodies == false` and so never reads
    /// it, which is what makes borrowing rather than copying safe -- and copying would
    /// cost a byte per instruction retained for the module's life, which is the opposite
    /// of what this whole path is for.
    const Int32* instRegionDuringLoadWalk = nullptr;

    /// True if instruction `index` is part of the eager skeleton, as decided by
    /// `_computeDeferralRegions`. Only meaningful while the load walk is deferring.
    bool isEagerSkeletonInst(Int64 index) const
    {
        SLANG_ASSERT(instRegionDuringLoadWalk);
        return instRegionDuringLoadWalk[index] < 0;
    }

    /// Serialises deferred decoding.
    ///
    /// A decode mutates state global to the module -- the cursors, the instruction array,
    /// the arena -- so it is serialised wholesale rather than per instruction. Contention
    /// is limited to the first touch of each body.
    ///
    /// The concurrency guarded against is the supported one: the
    /// serial-frontend/parallel-backend workflow in docs/user-guide/08-compiling.md.
    /// `link()` leaves bodies it did not need encoded and emit walks them, so first
    /// touches happen on the concurrent side, and several threads can observe the
    /// deferred flag for one body before any finishes. `materializeDeferredBody` rechecks
    /// under this lock for exactly that case.
    std::mutex mutex;

    Int64 instIndex = 0;
    Int64 operandCursor = 0;
    Int64 literalCursor = 0;
    Int64 stringLengthCursor = 0;
    Int64 stringDataCursor = 0;

    /// The instruction array, indexed from -1 so that a serialized -1 reads as null.
    ///
    /// Asserted rather than assumed: the load path sizes `instsList` before the first
    /// call, but deferred materialization reaches this from paths that do not, and
    /// `&instsList[1]` on an empty list is out of bounds without saying so.
    IRInst** insts()
    {
        SLANG_RELEASE_ASSERT(instsList.getCount() >= 1);
        return &instsList[1];
    }
    Int64 getInstCount() const { return flat.instAllocInfo.getCount(); }

    /// Why an operand is being read, which decides whether a null result is a violation.
    ///
    /// A bare `true`/`false` at the call site said nothing about the distinction it
    /// selects, and the distinction is the load-bearing one: it is the difference between
    /// "this must resolve or the whole deferral scheme is unsound" and "a null here is
    /// the expected answer".
    enum class OperandUse
    {
        /// The result is about to be wired into a live instruction, so it must resolve.
        WireIntoLiveInst,
        /// The operand belongs to an instruction the walk deliberately skipped, and is
        /// read only to keep the cursors aligned. Such operands may name other skipped
        /// instructions, where null is the correct result.
        ConsumeForSkippedInst,
    };

    /// Reads one operand index and resolves it to an instruction.
    ///
    /// Advances the operand cursor by one either way; the `use` only decides how strictly
    /// the result is checked.
    IRInst* readInstRef(OperandUse use)
    {
        const bool mustResolve = (use == OperandUse::WireIntoLiveInst);
        SLANG_RELEASE_ASSERT(operandCursor < flat.operandIndices.getCount());
        const auto index = flat.operandIndices[operandCursor++];
        SLANG_RELEASE_ASSERT(index >= -1 && index < getInstCount());
        IRInst* const result = insts()[index];
        // -1 encodes a null operand. Anything else must resolve to an instruction that
        // exists, which is the invariant the whole scheme rests on: nothing outside a
        // deferred body refers into one, so no eagerly decoded operand can land on a
        // slot the skeleton left empty. Measured as holding across every operand in the
        // builtin modules; assert it rather than return the null and fail later.
        SLANG_RELEASE_ASSERT(!mustResolve || index == -1 || result);
        return result;
    }

    /// Decodes the instruction at the cursor and, recursively, its children.
    ///
    /// **Advances every cursor it touches** -- instruction, operand, literal, string
    /// length and string data. The payload for instruction *i* is "the next unread
    /// entry" rather than something addressable by index, so the cursors are the only
    /// thing saying where the next instruction's data begins; that is why
    /// `materializeDeferredBody` restores all five before replaying a subtree.
    ///
    /// A null return means the instruction was deliberately not materialized. Its
    /// payload entries are consumed anyway, to keep the cursors aligned.
    IRInst* decodeInst(IRInst* parent, Int64 depth);

    /// The opcode to decode instruction `index` as, mapping an opcode this build does
    /// not know to `kIROp_Unrecognized` and recording that it happened.
    ///
    /// The single spelling of that mapping, used by both paths that allocate: the
    /// load-time pass and the deferred materialization that allocates the same
    /// instructions later. Having one also gives the flag one home -- a deferred decode
    /// runs with no `IRSerialReadContext` to reach, so it has to be recorded here and
    /// propagated once the load walk is done.
    IROp getInstOpAndNoteIfUnrecognized(Int64 index)
    {
        const IROp op = flat.instAllocInfo[index].op;
        if (op == kIROp_Invalid) [[unlikely]]
        {
            sawUnrecognizedOpDuringDecode = true;
            return kIROp_Unrecognized;
        }
        return op;
    }

    /// Allocates the instruction for a given index; see the definition.
    IRInst* allocateInstAt(Int64 instIndexToAlloc, Int64& ioStringLengthCursor);

    /// `IRDeferredBodyLoader`.
    void materializeDeferredBody(IRInst* inst) override;
};

void FlatModuleDecoder::materializeDeferredBody(IRInst* inst)
{
    std::lock_guard<std::mutex> lock(mutex);

    DeferredBody body;
    if (!deferredBodies.tryGetValue(inst, body))
    {
        // Another thread decoded this body while we waited for the lock.
        return;
    }
    // `body` is a copy, so the entry is not needed past this point -- but it is
    // removed only after the children are linked, below. Removing it here would
    // mean that a decode which aborts part-way (an assertion in this file throws
    // rather than terminating) leaves the instruction still flagged as deferred
    // with no entry to decode: the next access would find nothing, return quietly,
    // and hand the caller an empty body as though it were complete.

    // This replays the subtree from where the load walk left off, with deferral
    // disabled so nested instructions materialize in full. Every instruction in the
    // subtree is allocated before any of them are wired, matching the load-time path:
    // instructions forward-reference each other -- a branch names a block defined
    // later -- so an operand read before its target exists would silently resolve to
    // null.
    {
        // A private cursor for the sizing walk, named as at load time and deliberately
        // not the member: this pass runs ahead of the decode to size allocations, so
        // advancing the member here would leave it past the body's start before the
        // decode below rewinds it.
        Int64 allocStringLengthCursor = body.stringLengthCursor;
        const Int64 end = body.firstChildInstIndex + body.instCount;
        for (Int64 i = body.firstChildInstIndex; i < end; ++i)
        {
            if (!insts()[i])
                insts()[i] = allocateInstAt(i, allocStringLengthCursor);
        }
    }

    const bool savedDefer = deferBodies;
    deferBodies = false;
    instIndex = body.firstChildInstIndex;
    operandCursor = body.operandCursor;
    literalCursor = body.literalCursor;
    stringLengthCursor = body.stringLengthCursor;
    stringDataCursor = body.stringDataCursor;

    // Build the body as a detached chain first, then attach it with a single store.
    //
    // The children are unreachable by any other thread while they are being built, so
    // linking them to each other needs no synchronization. Attaching must then be the
    // only publication, and exactly one store: splicing the chain on as it is built --
    // linking the first child to the last decoration before the rest exist -- would let a
    // concurrent decoration walk follow that link into a chain still being decoded.
    IRInst* const lastDecoration = inst->peekLastDecorationOrChild();
    IRInst* bodyFirst = nullptr;
    IRInst* bodyLast = nullptr;
    for (Int64 i = 0; i < body.childCount; ++i)
    {
        auto child = decodeInst(inst, kBodyChildDepth);
        if (!child)
            continue;
        child->setPrevInst(bodyLast);
        if (bodyLast)
            bodyLast->setNextInst(child);
        else
            bodyFirst = child;
        bodyLast = child;
    }
    if (bodyLast)
        bodyLast->setNextInst(nullptr);

    if (bodyFirst)
    {
        bodyFirst->setPrevInst(lastDecoration);
        inst->setLastDecorationOrChild(bodyLast);
        // The publishing store. Release so that a reader which observes the link also
        // observes every field of every instruction in the chain behind it.
        if (lastDecoration)
            lastDecoration->setNextInst(bodyFirst);
        else
            inst->setFirstDecorationOrChild(bodyFirst);
    }

    deferBodies = savedDefer;

    // Drop the entry and clear the flag together, so the two never disagree.
    deferredBodies.remove(inst);

    // Release: a thread that later observes this as false must also see every
    // write above, so that it reads a fully linked body.
    inst->m_hasDeferredBody.store(false, std::memory_order_release);
}


/// Takes the minimum total allocation size for an instruction of `op`, **advancing
/// `ioStringLengthCursor`** past the length entry of a string or blob constant.
///
/// Named `take` rather than `read`/`get` because the cursor advance is the point, not a
/// detail: calling it twice for one instruction, or without threading the caller's own
/// cursor, shifts every subsequent string constant by one entry.
///
/// The result is an absolute floor, not an increment: `_allocateInst` takes the larger of
/// it and `sizeof(IRInst) + operandCount * sizeof(IRUse)`, and `0` means "no floor".
/// The cursor advance is a stream side effect both callers depend on happening exactly
/// once, which is why each threads its own cursor in by reference.
///
/// Shared by the load-time walk and by deferred materialization, which must size the
/// same instruction identically. Keeping one copy also keeps the two range checks below
/// on both paths; they are what stop a corrupt or future-version table from truncating
/// `numChars` or overflowing the allocation the subsequent `memcpy` writes into, and a
/// duplicate of this switch is easy to write without them.
static size_t _takeInstMinSizeInBytes(
    IROp op,
    const FlatInstTable& flat,
    Int64& ioStringLengthCursor)
{
    switch (op)
    {
    [[unlikely]] case kIROp_ModuleInst:
        return offsetof(IRModuleInst, module) +
               sizeof(IRModuleInst::module); // NOLINT(bugprone-sizeof-expression)
    case kIROp_BoolLit:
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_PtrLit:
    case kIROp_VoidLit:
        return offsetof(IRConstant, value) + sizeof(IRConstant::value);
    // About 5% of instructions in the core module are strings!
    case kIROp_StringLit:
    case kIROp_BlobLit:
        {
            SLANG_RELEASE_ASSERT(ioStringLengthCursor < flat.stringLengths.getCount());
            const auto len = flat.stringLengths[ioStringLengthCursor++];
            SLANG_RELEASE_ASSERT(len >= 0);
            // `IRConstant::StringValue::numChars` is `uint32_t`; a longer length would
            // truncate when it is stored.
            SLANG_RELEASE_ASSERT(uint64_t(len) <= uint64_t(UINT32_MAX));

            const size_t headerSize =
                offsetof(IRConstant, value) + offsetof(IRConstant::StringValue, chars);
            // Guard the addition itself, so a huge length cannot wrap and yield an
            // allocation smaller than the characters later copied into it.
            SLANG_RELEASE_ASSERT(size_t(len) <= size_t(-1) - headerSize);

            return headerSize + size_t(len);
        }
    default:
        return 0;
    }
}

/// Allocates the instruction for `instIndex`, mirroring the sizing rules of the
/// load-time allocation pass.
///
/// Needed because a deferred body's instructions were never allocated: the load
/// pass left their slots empty. String and blob constants carry their characters
/// inline, so their size depends on a length that is read from the payload stream;
/// the cursor is positioned at that length here, and reading it **advances** the cursor
/// past it. That is why the caller passes its own cursor by reference and why the payload
/// switch below does not read the length again -- a second read, or removing this one as
/// redundant, shifts every subsequent string constant by one entry.
IRInst* FlatModuleDecoder::allocateInstAt(Int64 instIndexToAlloc, Int64& ioStringLengthCursor)
{
    const auto& allocInfo = flat.instAllocInfo[instIndexToAlloc];
    const IROp op = getInstOpAndNoteIfUnrecognized(instIndexToAlloc);
    const size_t minSizeInBytes = _takeInstMinSizeInBytes(op, flat, ioStringLengthCursor);
    return module->_allocateInst(op, allocInfo.operandCount, minSizeInBytes);
}


IRInst* FlatModuleDecoder::decodeInst(IRInst* parent, Int64 depth)
{
    SLANG_RELEASE_ASSERT(depth < kMaxIRSerializationDepth);
    SLANG_RELEASE_ASSERT(instIndex < getInstCount());

    const auto thisInstIndex = instIndex++;
    IRInst* inst = insts()[thisInstIndex];

    // Under on-demand load this instruction may have been skipped. Its operand and
    // payload entries still have to be consumed so the cursors stay aligned for
    // the instructions that were kept.
    const auto& allocInfo = flat.instAllocInfo[thisInstIndex];

    // The table is the single source for how many operands to consume. When the
    // instruction exists it was allocated with this same count, so the two agree by
    // construction -- but if they ever stopped agreeing, reading the instruction's
    // count would desynchronize the operand cursor for every instruction after this
    // one, and the damage would surface nowhere near here.
    const Int64 thisOperandCount = Int64(allocInfo.operandCount);
    SLANG_ASSERT(!inst || Int64(inst->operandCount) == thisOperandCount);

    // operands and sourcelocs
    if (inst)
    {
        inst->sourceLoc = flat.sourceLocs[thisInstIndex];
        inst->typeUse.init(inst, readInstRef(OperandUse::WireIntoLiveInst));
        for (Int64 o = 0; o < thisOperandCount; ++o)
            inst->getOperands()[o].init(inst, readInstRef(OperandUse::WireIntoLiveInst));
    }
    else
    {
        readInstRef(OperandUse::ConsumeForSkippedInst); // type use
        for (Int64 o = 0; o < thisOperandCount; ++o)
            readInstRef(OperandUse::ConsumeForSkippedInst);
    }

    // Handle special instructions
    switch (inst ? inst->m_op : allocInfo.op)
    {
    [[unlikely]] case kIROp_ModuleInst:
        if (inst)
            cast<IRModuleInst>(inst)->module = module;
        break;
    case kIROp_BoolLit:
    case kIROp_IntLit:
        {
            SLANG_RELEASE_ASSERT(literalCursor < flat.literals.getCount());
            const auto bits = flat.literals[literalCursor++];
            if (inst)
                cast<IRConstant>(inst)->value.intVal = bitCast<IRIntegerValue>(bits);
            break;
        }
    case kIROp_FloatLit:
        {
            SLANG_RELEASE_ASSERT(literalCursor < flat.literals.getCount());
            const auto bits = flat.literals[literalCursor++];
            if (inst)
                cast<IRConstant>(inst)->value.floatVal = bitCast<double>(bits);
            break;
        }
    case kIROp_PtrLit:
        {
            SLANG_RELEASE_ASSERT(literalCursor < flat.literals.getCount());
            const auto bits = flat.literals[literalCursor++];
            // Keep the compiler happy on 32 bit builds
            if (inst)
                cast<IRConstant>(inst)->value.ptrVal = (void*)(uintptr_t(bits));
            break;
        }
    case kIROp_StringLit:
    case kIROp_BlobLit:
        {
            auto* const c = inst ? cast<IRConstant>(inst) : nullptr;
            SLANG_RELEASE_ASSERT(stringLengthCursor < flat.stringLengths.getCount());
            const auto len = flat.stringLengths[stringLengthCursor++];
            SLANG_RELEASE_ASSERT(len >= 0);
            SLANG_RELEASE_ASSERT(uint64_t(len) <= uint64_t(UINT32_MAX));

            const auto stringCharsCount = flat.stringChars.getCount();
            SLANG_RELEASE_ASSERT(stringDataCursor <= stringCharsCount);
            SLANG_RELEASE_ASSERT(len <= stringCharsCount - stringDataCursor);

            if (c)
            {
                char* const dstChars = c->value.stringVal.chars;
                c->value.stringVal.numChars = uint32_t(len);
                if (len != 0)
                    memcpy(dstChars, flat.stringChars.getBuffer() + stringDataCursor, size_t(len));
            }
            stringDataCursor += len;
            break;
        }
    }

    // Read in children, and fix up pointers. Children that were skipped come
    // back as null and are simply not linked, which is what leaves a global
    // value holding its decorations but no body.
    if (inst)
        inst->parent = parent;
    IRInst* prev = nullptr;
    IRInst* first = nullptr;
    IRInst* last = nullptr;
    const auto childCount = flat.childCounts[thisInstIndex];
    SLANG_RELEASE_ASSERT(childCount >= 0);
    for (Int64 i = 0; i < childCount; ++i)
    {
        // Where a global's body starts is not decided here: `_computeDeferralRegions`
        // decided it for the whole module, and the first child it left out of the eager
        // skeleton is the first instruction of the body. Note where that body's encoding
        // starts, then let the remaining children be walked without being materialized --
        // the walk still has to run, to consume their operand and payload entries and
        // keep the cursors aligned.
        // `!inst->m_hasDeferredBody` latches: it is set a few lines below, so the
        // deferral branch fires once per global, on its first non-eager child. A plain
        // read is right here even though this field carries acquire/release ordering
        // elsewhere -- the load walk is single-threaded and runs before the decoder is
        // reachable by anyone else.
        if (deferBodies && depth == kGlobalValueDepth && inst && !inst->m_hasDeferredBody)
        {
            // Looked at before the recursive call validates it, so bound it here; a
            // corrupt `childCounts` is what would put this out of range.
            SLANG_RELEASE_ASSERT(instIndex < getInstCount());
            if (!isEagerSkeletonInst(instIndex))
            {
                DeferredBody body;
                body.firstChildInstIndex = instIndex;
                body.childCount = childCount - i;
                body.operandCursor = operandCursor;
                body.literalCursor = literalCursor;
                body.stringLengthCursor = stringLengthCursor;
                body.stringDataCursor = stringDataCursor;
                deferredBodies.add(inst, body);
                // Plain (seq_cst) store, unlike the release store that clears this flag
                // in `materializeDeferredBody`, and deliberately so: setting it happens
                // during the load walk, before the module or this decoder is reachable
                // from any other thread, so there is nothing to synchronize with yet.
                // The orderings elsewhere on this field are load-bearing; this one is
                // not, and the asymmetry is intentional rather than an oversight.
                inst->m_hasDeferredBody = true;
            }
        }
        auto c = decodeInst(inst, depth + 1);
        if (!c)
            continue;
        if (!first)
            first = c;
        last = c;
        c->setPrevInst(prev);
        if (prev)
            prev->setNextInst(c);
        prev = c;
    }
    if (last)
        last->setNextInst(nullptr);
    if (inst)
    {
        inst->setFirstDecorationOrChild(first);
        inst->setLastDecorationOrChild(last);
    }

    // Now that the whole subtree has been walked, record how many instructions
    // it spans, so materializing it later can pre-allocate them all.
    if (inst && inst->m_hasDeferredBody)
    {
        // The entry was added on the deferral branch above and `m_hasDeferredBody` is
        // the guard, so this lookup cannot miss. Assert rather than skip: leaving
        // `instCount` at 0 would make materialization pre-allocate nothing while the
        // decode still walks every child, so forward references would resolve against
        // null slots and fail somewhere far away.
        auto recorded = deferredBodies.tryGetValue(inst);
        SLANG_RELEASE_ASSERT(recorded);
        recorded->instCount = instIndex - recorded->firstChildInstIndex;
    }

    return inst;
}

static IRModuleInst* deserializeFromFlatModule(const IRReadSerializer& serializer, IRModule* module)
{
    IRSerialReadContext& readContext = *serializer.getContext();
    RefPtr<FlatModuleDecoder> decoder = new FlatModuleDecoder();
    decoder->module = module;
    FlatInstTable& flat = decoder->flat;
    serialize(serializer, flat);
    const List<SourceLoc>& sourceLocs = flat.sourceLocs;
    // dumpFlatInstTableStats(flat, "deserializing");

    List<IRInst*>& instsList = decoder->instsList;

    // Pass 1 walks the string lengths independently of the decoding cursors below,
    // purely to size the allocations for string and blob constants. Named apart from
    // the decoder's `stringLengthCursor` deliberately: this one runs to completion here
    // and is then done, while the decoder's is saved and restored across deferred
    // decodes, so conflating the two would hide that only one of them is replayed.
    Int64 allocStringLengthCursor = 0;

    const auto numInsts = flat.instAllocInfo.getCount();

    const auto operandIndicesCount = flat.operandIndices.getCount();

    // These relationships are serialized IR invariants; stop before rebuilding pointers from
    // inconsistent flat tables.
    SLANG_RELEASE_ASSERT(flat.childCounts.getCount() == numInsts);
    SLANG_RELEASE_ASSERT(sourceLocs.getCount() == numInsts);

    instsList.setCount(numInsts + 1);
    // nullptr instructions are represented as `-1`. We can save ourselves a
    // branch by just making that index valid.
    IRInst** const insts = decoder->insts();
    insts[-1] = nullptr;

    // An on-demand load materializes only what a symbol index needs -- the module inst,
    // each module-scope global, and each global's decorations -- and leaves each
    // global's body encoded until something asks for its children.
    //
    // This needs no change to the serialized format. The obstacle to decoding one
    // instruction on its own is that operands, literals and strings are consumed by
    // running cursors in preorder; those cursor positions are recovered here by a
    // scan over `childCounts` that allocates nothing.
    // Deferral is only sound when something keeps the serialized bytes alive: the flat
    // table spans point into them, and a body is decoded long after this returns. A
    // caller that reads out of its own buffer supplies no blob, and gets an eager load.
    decoder->blobHoldingSerializedData = readContext.getBlobHoldingSerializedData();
    bool onDemandIRLoad = decoder->blobHoldingSerializedData != nullptr;

    // Deferral is only safe if the blob is the storage these spans were parsed out of, not
    // merely a blob the caller happened to have. Otherwise a body is decoded out of freed
    // memory, silently and long after the call that would be blamed for it.
    //
    // What is actually testable here is containment -- that every view's bytes lie within
    // the blob's range -- which is a proxy for that, and the names below say containment
    // rather than ownership so the two are not confused. That is not
    // hypothetical: `addLibraryReference` retained a copy while parsing the caller's
    // pointer, which was harmless until bodies stopped being materialized eagerly.
    if (onDemandIRLoad)
    {
        const Byte* const blobBegin =
            (const Byte*)decoder->blobHoldingSerializedData->getBufferPointer();
        const Byte* const blobEnd = blobBegin + decoder->blobHoldingSerializedData->getBufferSize();
        const uintptr_t blobLow = (uintptr_t)blobBegin;
        const uintptr_t blobHigh = (uintptr_t)blobEnd;

        // Integer comparison, not pointer comparison. This runs precisely when a span may
        // point into a *different* allocation, and there `<`/`>=` is unspecified
        // ([expr.rel]) and forming `data + size` is undefined ([expr.add]) -- so the
        // pointer spelling would be reasoning the optimizer may discard, in the one case
        // the guard exists for. `p <= hi` is established before `hi - p` is evaluated, and
        // the size is compared against that difference rather than added to `p`, so
        // nothing overflows.
        auto spanIsInsideBlob = [&](const Byte* data, uintptr_t sizeInBytes)
        {
            const uintptr_t p = (uintptr_t)data;
            if (p < blobLow || p > blobHigh)
                return false;
            return sizeInBytes <= blobHigh - p;
        };

        // The byte size is computed in 64 bits on every target. On wasm32 `Count` and
        // `uintptr_t` are both 32 bits, so `count * elementSize` wraps above ~2^29: a
        // corrupt count of 0x20000001 with an 8-byte stride would wrap to 8 and pass the
        // containment check below. Nothing validates the count before this point --
        // `_pushContainerState` takes it verbatim from the container header.
        //
        // The element size comes from the array's own element type rather than a
        // parameter, so a later type change cannot silently invalidate the check.
        auto arrayIsInsideBlob = [&]<typename T>(SerializedArray<T> const& array)
        {
            constexpr uint64_t elementSize = sizeof(T);

            if (!array.isView())
                return true;
            const Count count = array.getCount();
            if (count < 0)
                return false;
            const uint64_t elementCount = (uint64_t)count;
            // Refuse rather than wrap: on a 64-bit target a large count times a stride
            // can still exceed 64 bits.
            if (elementCount > UINT64_MAX / elementSize)
                return false;
            const uint64_t byteSize = elementCount * elementSize;
            // A span wider than the address space cannot be inside the blob, and must not
            // be narrowed on the way into the check.
            if (byteSize > (uint64_t)UINTPTR_MAX)
                return false;
            return spanIsInsideBlob((const Byte*)array.getBuffer(), (uintptr_t)byteSize);
        };

        // Every view-capable array, not a sample: which ones are views depends on the
        // backend and on what the module contains, so a subset check passes whenever the
        // arrays it named happened to be the owned ones.
        const bool everySpanIsInsideBlob =
            arrayIsInsideBlob(flat.childCounts) && arrayIsInsideBlob(flat.operandIndices) &&
            arrayIsInsideBlob(flat.stringLengths) && arrayIsInsideBlob(flat.stringChars) &&
            arrayIsInsideBlob(flat.literals);

        if (!everySpanIsInsideBlob)
        {
            // Fall back to an eager load rather than asserting. A caller that supplies an
            // unrelated blob then gets correct behaviour at the old cost, which is a better
            // failure mode than aborting a compile -- and eager loading is exactly what this
            // path did before deferral existed.
            onDemandIRLoad = false;
        }
    }
    // Which instructions are eager skeleton and which are deferrable body, decided once
    // for the whole module. Both readers below -- the allocation pass and, through
    // `isEagerSkeletonInst`, the decode walk -- work off this one answer.
    List<Int32> instRegion;
    if (onDemandIRLoad)
    {
        _computeDeferralRegions(flat, numInsts, instRegion);
        // Costs one `Int32` per instruction for the duration of the load, and one pass
        // over the operand table. Both are transient; what they buy is that a module
        // violating the invariant loads slowly instead of aborting.
        if (!_deferralRegionsAreClosed(flat, numInsts, instRegion))
            onDemandIRLoad = false;
    }

    for (Int64 instIndex = 0; instIndex < numInsts; ++instIndex)
    {
        const auto& a = flat.instAllocInfo[instIndex];
        const IROp op = decoder->getInstOpAndNoteIfUnrecognized(instIndex);
        const size_t minSizeInBytes = _takeInstMinSizeInBytes(op, flat, allocStringLengthCursor);
        // Under on-demand load the skipped instructions are never allocated; the
        // preorder walk below still consumes their operand and payload cursors so
        // that positions stay correct for the instructions that are kept.
        insts[instIndex] = (onDemandIRLoad && instRegion[instIndex] >= 0)
                               ? nullptr
                               : module->_allocateInst(op, a.operandCount, minSizeInBytes);
    }

    decoder->deferBodies = onDemandIRLoad;
    decoder->instRegionDuringLoadWalk = onDemandIRLoad ? instRegion.getBuffer() : nullptr;
    const auto moduleInst = decoder->decodeInst(nullptr, kModuleInstDepth);
    // The borrow ends with the walk. `instRegion` is a local of this function, and the
    // decoder outlives it whenever any body was deferred.
    decoder->instRegionDuringLoadWalk = nullptr;

    // The decoder must stay alive so the bodies it skipped can still be decoded later: it
    // holds the flat table and the instruction array a body needs, since a body's
    // operands are indices into that array and may name any module-scope global.
    if (decoder->deferredBodies.getCount())
    {
        module->setDeferredBodyLoader(decoder);
    }

    // The walk visits every instruction and consumes every payload entry even when
    // bodies are deferred -- deferring skips materialization, not traversal -- so
    // these end-state checks hold either way.
    SLANG_RELEASE_ASSERT(decoder->instIndex == numInsts);
    SLANG_RELEASE_ASSERT(decoder->operandCursor == operandIndicesCount);
    // Unknown future opcodes intentionally become a recoverable read failure later.
    // This reader cannot know whether those opcodes consume literal or string payloads.
    //
    // Everything that allocates records this on the decoder, since a body decoded after
    // this function returns has no context to reach. Propagate it here, while the context
    // is still alive and before the end-state checks below consult it.
    readContext._foundUnrecognizedInstructions |= decoder->sawUnrecognizedOpDuringDecode;

    if (!readContext._foundUnrecognizedInstructions)
    {
        SLANG_RELEASE_ASSERT(decoder->literalCursor == flat.literals.getCount());
        SLANG_RELEASE_ASSERT(decoder->stringLengthCursor == flat.stringLengths.getCount());
        SLANG_RELEASE_ASSERT(decoder->stringDataCursor == flat.stringChars.getCount());
    }
    SLANG_RELEASE_ASSERT(as<IRModuleInst>(moduleInst));
    return cast<IRModuleInst>(moduleInst);
}

void IRSerialWriteContext::handleIRModule(IRWriteSerializer const& serializer, IRModule*& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value->m_name);
    serialize(serializer, value->m_version);
    serializeAsFlatModule(serializer, value->m_moduleInst);
}

void IRSerialReadContext::handleIRModule(IRReadSerializer const& serializer, IRModule*& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    value = new IRModule{_session};
    SLANG_ASSERT(!_module);
    _module = value;
    serialize(serializer, value->m_name);
    serialize(serializer, value->m_version);
    value->m_moduleInst = deserializeFromFlatModule(serializer, value);
}

//
// {write,read}SerializedModuleIR()
//

void writeSerializedModuleIR(
    RIFF::BuildCursor& cursor,
    IRModule* irModule,
    SerialSourceLocWriter* sourceLocWriter)
{
    // The flow here is very similar to writeSerializedModuleAST which is very
    // well documented.

    IRModuleInfo moduleInfo;
    moduleInfo.fullVersion = SLANG_TAG_VERSION;
    moduleInfo.module = irModule;

    BlobBuilder blobBuilder;
    {
        // Note: `context` must be declared before `writer` so that it outlives
        // it; ~SerialWriter flushes deferred writes that call back into the
        // context.
        IRSerialWriteContext context{sourceLocWriter};
        Fossil::SerialWriter writer(blobBuilder);
        IRWriteSerializer serializer(&writer, &context);
        serialize(serializer, moduleInfo);
    }

    ComPtr<ISlangBlob> blob;
    blobBuilder.writeToBlob(blob.writeRef());

    void const* data = blob->getBufferPointer();
    size_t size = blob->getBufferSize();
    cursor.addDataChunk(PropertyKeys<IRModule>::IRModule, data, size);
}

Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String& compilerVersion,
    UInt& version,
    String& name)
{
    auto dataChunk = as<RIFF::DataChunk>(chunk);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format for serialized module IR");
    }

    Fossil::AnyValPtr rootValPtr =
        Fossil::getRootValue(dataChunk->getPayload(), dataChunk->getPayloadSize());
    if (!rootValPtr)
    {
        SLANG_UNEXPECTED("invalid format for serialized module IR");
    }

    Fossilized<IRModuleInfo>* fossilizedModuleInfo = cast<Fossilized<IRModuleInfo>>(rootValPtr);
    Fossilized<IRModule>* fossilizedModule = fossilizedModuleInfo->module;
    version = fossilizedModule->m_version;
    compilerVersion = fossilizedModuleInfo->fullVersion.get();
    name = fossilizedModuleInfo->module->m_name.get();
    return SLANG_OK;
}

// A helper to make profiling the actual deserialization work
// easier.
[[nodiscard]] static Result readSerializedModuleIR_(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    ISlangBlob* blobHoldingSerializedData,
    RefPtr<IRModule>& outIRModule)
{
    auto dataChunk = as<RIFF::DataChunk>(chunk);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format for serialized module IR");
    }

    Fossil::AnyValPtr rootValPtr =
        Fossil::getRootValue(dataChunk->getPayload(), dataChunk->getPayloadSize());
    if (!rootValPtr)
    {
        SLANG_UNEXPECTED("invalid format for serialized module IR");
    }

    Fossilized<IRModuleInfo>* fossilizedModuleInfo = cast<Fossilized<IRModuleInfo>>(rootValPtr);

    // Only one version supported so far, if we had multiple versions to
    // support this is where we might branch
    if (fossilizedModuleInfo->serializationVersion != IRModuleInfo::kSupportedSerializationVersion)
        return SLANG_FAIL;

    IRModuleInfo info;
    auto sharedDecodingContext =
        RefPtr(new IRSerialReadContext(session, sourceLocReader, blobHoldingSerializedData));
    {
        Fossil::ReadContext readContext;
        Fossil::SerialReader reader(
            readContext,
            rootValPtr,
            Fossil::SerialReader::InitialStateType::Root);

        IRReadSerializer serializer(&reader, sharedDecodingContext);
        serialize(serializer, info);
    }
    if (!info.module)
        return SLANG_FAIL;
    outIRModule = info.module;
    if (sharedDecodingContext->_foundUnrecognizedInstructions)
        return SLANG_FAIL;
    return SLANG_OK;
}

Result readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    ISlangBlob* blobHoldingSerializedData,
    RefPtr<IRModule>& outIRModule)
{
    SLANG_PROFILE;

    SLANG_RETURN_ON_FAIL(readSerializedModuleIR_(
        chunk,
        session,
        sourceLocReader,
        blobHoldingSerializedData,
        outIRModule));

    //
    // Module is finally valid (or at least as much as it was going it) and
    // ready to be used
    //
    outIRModule->buildMangledNameToGlobalInstMap();

    return SLANG_OK;
}

} // namespace Slang
