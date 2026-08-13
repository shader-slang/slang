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
    ComPtr<ISlangBlob> _blobHoldingSerializedData;
    virtual void handleIRModule(IRReadSerializer const& serializer, IRModule*& value);
    virtual void handleName(IRReadSerializer const& serializer, Name*& value);
    virtual SerialSourceLocReader* getSourceLocReader() override { return _sourceLocReader; }

    // Used to allocate an IRModule
    Session* _session;

    //
    SerialSourceLocReader* _sourceLocReader;

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

//
/// True if builtin-module instruction bodies should be left encoded until something
/// reads them. **On by default**; `SLANG_ONDEMAND_IR=0` forces the eager load.
///
/// The override exists because the two paths must produce identical results, and the
/// cheapest way to investigate a suspected difference is to run the same binary both
/// ways. It is deliberately an override rather than an opt-in: shipping this off by
/// default would mean nobody gets the reduction without knowing to ask, and the
/// deferred path would go untested in ordinary runs.
///
/// Read once: a global session is shared across threads, and the underlying
/// environment lookup is not safe against a concurrent write. Uses
/// `PlatformUtil::getEnvironmentVariable` rather than `getenv`, which MSVC
/// deprecates and this build treats as an error.
bool isOnDemandIRLoadEnabled()
{
    static const bool enabled = []
    {
        StringBuilder value;
        if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(
                UnownedStringSlice("SLANG_ONDEMAND_IR"),
                value)))
        {
            return true;
        }
        // Set-but-empty reads as "not specified", so that clearing the variable in a
        // shell behaves the same as never having set it.
        const String text = value.produceString();
        return text.getLength() == 0 || text[0] != '0';
    }();
    return enabled;
}

// Decoding state for a module's flat instruction table.
//
// The same walk serves two purposes, which is why it lives in an object rather
// than a lambda: it runs once over the whole module at load time, and then again
// over a single subtree each time a deferred body is asked for. Holding the flat
// table and the instruction array keeps the second use possible -- a body's
// operands are indices into that array, and may name any module-scope global.
//
// Depths in the module's preorder walk. The module inst is the root, its globals sit
// directly under it, and a global's decorations and body children sit under those. Three
// separate pieces of logic depend on this model agreeing -- the deferral test in
// `decodeInst`, the eager-skeleton scan, and the depth a replayed body is decoded at --
// so the numbers are named rather than written out at each site.
static const Int64 kModuleInstDepth = 0;
static const Int64 kGlobalValueDepth = 1;
static const Int64 kBodyChildDepth = 2;

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

    /// Set when an unrecognized opcode is seen, by either the load walk or a body
    /// decoded later.
    ///
    /// Recorded here rather than on the `IRSerialReadContext`, which this must not
    /// reference: the context holds a `RefPtr` to the `IRModule`, the module holds
    /// this decoder, and a strong reference from here would close that loop and leak
    /// the module, this decoder's flat table and instruction array, and the retained
    /// blob -- for every module loaded, for the life of the process. A raw pointer
    /// would instead dangle, since the context is owned by a `RefPtr` local to the
    /// load. The load propagates this into the context while that context is still
    /// alive; a deferred decode has no reader for it, and records it here so the
    /// information is at least not written into freed memory.
    bool foundUnrecognizedInstructions = false;

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

    /// True while the load walk should defer bodies. Cleared during a deferred
    /// decode so that nested subtrees materialize fully.
    ///
    /// The flag means two different things at two different times: during the initial
    /// load walk it is the top-level on-demand/eager mode, and during a later
    /// `materializeDeferredBody` it is forced false and restored on the way out.
    ///
    /// **Not reentrancy-safe on its own.** Because it is a member rather than a
    /// parameter threaded through `decodeInst`, the save/restore is only correct while
    /// no second decode can interleave with it -- which holds today because every
    /// deferred decode runs under `mutex`, and the load walk runs before the decoder is
    /// reachable by anyone else. A future caller that reaches `decodeInst` from some
    /// other context must take that lock or thread the mode through as a parameter;
    /// otherwise one decode's restore will overwrite another's mode mid-walk and
    /// bodies will be deferred, or not, at the wrong depth.
    bool deferBodies = false;

    /// Serialises deferred decoding.
    ///
    /// The decode mutates state that is global to the module -- the cursors, the
    /// instruction array and the module's arena -- so it is serialised wholesale
    /// rather than per instruction. Contention is limited to the first touch of
    /// each body.
    ///
    /// The concurrency this guards against is the one Slang actually supports.
    /// Running whole compiles against a shared global session is *not* supported --
    /// `include/slang.h` says a global session is not thread-safe and that
    /// front-end work must be externally synchronized -- so that is not the
    /// justification. What is supported, per the serial-frontend/parallel-backend
    /// workflow in docs/user-guide/08-compiling.md, is calling `getEntryPointCode`
    /// and friends concurrently on a linked component type. Those run target
    /// passes and emit over IR that can still reference a builtin module, so a
    /// body can be first touched from several backend threads at once.
    ///
    /// That path is where the materializing actually happens, measured rather than
    /// assumed. Counting calls into the loader's slow path across the two phases of
    /// that workflow, for one compute entry point:
    ///
    ///     threads   during serial front end   during parallel backend
    ///        1                 0                        38
    ///        4                 0                        40
    ///        8                 0                        52
    ///       16                 0                        57
    ///
    /// The front end materializes nothing: linking leaves every body it did not
    /// need still encoded, and emit is what walks them. So every first touch happens
    /// on the concurrent side.
    ///
    /// The rise from 38 to 57 is the contended case occurring, not extra work being
    /// done. 38 is the number of distinct bodies; the excess is threads that all
    /// observed the deferred flag before any of them had finished, each entering the
    /// slow path for the same body. That is the shape `materializeDeferredBody`
    /// documents and handles by rechecking under this lock, and it is why the lock
    /// is load-bearing rather than insurance.
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
    /// length and string data -- for the subtree it walks. That is the central side
    /// effect and the reason the decode order is fixed: the payload for instruction *i*
    /// is "the next unread entry", not something addressable by index, so the cursors
    /// are the only thing that says where the next instruction's data begins.
    ///
    /// A null return means the instruction was deliberately not materialized. Its
    /// operand and payload entries are consumed anyway, precisely so that the cursors
    /// stay aligned for the instructions that are kept.
    IRInst* decodeInst(IRInst* parent, Int64 depth);

    /// Allocates the instruction for a given index; see the definition.
    IRInst* allocateInstAt(Int64 instIndexToAlloc, Int64& stringLengthCursor);

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

    // Replay this subtree from where the load walk left off, with deferral
    // disabled so nested instructions materialize in full.
    // Allocate every instruction in the subtree before wiring any of them, exactly
    // as the load-time path does. Instructions forward-reference each other --
    // a branch names a block defined later -- so an operand read before its target
    // exists would silently resolve to null.
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
    // linking them to each other needs no synchronization. Attaching is the only
    // publication, and it is one store: previously the chain was spliced onto the last
    // decoration on the first iteration, which let a concurrent decoration walk follow
    // that link into a chain that was still being decoded.
    IRInst* const lastDecoration = inst->m_decorationsAndChildren.last;
    IRInst* bodyFirst = nullptr;
    IRInst* bodyLast = nullptr;
    for (Int64 i = 0; i < body.childCount; ++i)
    {
        auto child = decodeInst(inst, kBodyChildDepth);
        if (!child)
            continue;
        child->prev = bodyLast;
        if (bodyLast)
            bodyLast->next = child;
        else
            bodyFirst = child;
        bodyLast = child;
    }
    if (bodyLast)
        bodyLast->next = nullptr;

    if (bodyFirst)
    {
        bodyFirst->prev = lastDecoration;
        inst->m_decorationsAndChildren.last = bodyLast;
        // The publishing store. Release so that a reader which observes the link also
        // observes every field of every instruction in the chain behind it.
        if (lastDecoration)
            irPublishInstLink(lastDecoration->next, bodyFirst);
        else
            irPublishInstLink(inst->m_decorationsAndChildren.first, bodyFirst);
    }

    deferBodies = savedDefer;

    // Drop the entry and clear the flag together, so the two never disagree.
    deferredBodies.remove(inst);

    // Release: a thread that later observes this as false must also see every
    // write above, so that it reads a fully linked body.
    inst->m_hasDeferredBody.store(false, std::memory_order_release);
}


/// Returns the allocation size an instruction of `op` needs beyond the base `IRInst`,
/// advancing `stringLengthCursor` past the length entry of a string or blob constant.
///
/// Shared by the load-time walk and by deferred materialization so the two cannot
/// drift: an earlier version of the deferred path duplicated this switch and, in
/// doing so, dropped both range checks below, which are what keep a corrupt or
/// future-version table from truncating `numChars` or overflowing the allocation
/// size that the subsequent `memcpy` writes into.
static size_t _readInstMinSizeInBytes(IROp op, const FlatInstTable& flat, Int64& stringLengthCursor)
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
            SLANG_RELEASE_ASSERT(stringLengthCursor < flat.stringLengths.getCount());
            const auto len = flat.stringLengths[stringLengthCursor++];
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
/// the cursor is positioned at that length here, and reading it *advances* the cursor
/// past it -- `flat.stringLengths[stringLengthCursor++]`. That is why the caller passes
/// its own cursor by reference and why the payload switch below does not read the length
/// again. An earlier version of this comment claimed the read "does not disturb" the
/// cursor, which is the opposite of what it does; anyone who believed it and added a
/// second read, or removed this one as redundant, would have shifted every subsequent
/// string constant by one entry.
IRInst* FlatModuleDecoder::allocateInstAt(Int64 instIndexToAlloc, Int64& stringLengthCursor)
{
    const auto& allocInfo = flat.instAllocInfo[instIndexToAlloc];
    IROp op = allocInfo.op;
    if (op == kIROp_Invalid) [[unlikely]]
    {
        // Report it the same way the load-time walk does. Without this a lazily
        // materialized module would silently accept an opcode that an eager load
        // reports, and the end-state checks keyed on this flag would not relax.
        op = kIROp_Unrecognized;
        foundUnrecognizedInstructions = true;
    }

    const size_t minSizeInBytes = _readInstMinSizeInBytes(op, flat, stringLengthCursor);
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
        // A global value's decorations come first and stay eager; everything
        // after them is its body. Note where that body's encoding starts, then
        // let the remaining children be walked without being materialized --
        // the walk still has to run, to consume their operand and payload
        // entries and keep the cursors aligned.
        if (deferBodies && depth == kGlobalValueDepth && inst && !inst->m_hasDeferredBody)
        {
            // Looked at before the recursive call validates it, so bound it here; a
            // corrupt `childCounts` is what would put this out of range.
            SLANG_RELEASE_ASSERT(instIndex < getInstCount());
            const IROp nextOp = flat.instAllocInfo[instIndex].op;
            const bool nextIsDecoration =
                nextOp >= kIROp_FirstDecoration && nextOp <= kIROp_LastDecoration;
            if (!nextIsDecoration)
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
        c->prev = prev;
        if (prev)
            prev->next = c;
        prev = c;
    }
    if (last)
        last->next = nullptr;
    if (inst)
    {
        inst->m_decorationsAndChildren.first = first;
        inst->m_decorationsAndChildren.last = last;
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
    bool onDemandIRLoad = isOnDemandIRLoadEnabled() && decoder->blobHoldingSerializedData;

    // The blob must be the storage the data was actually parsed out of, not merely a blob
    // the caller happened to have. If it is not, the spans below point somewhere the blob
    // does not keep alive, and a deferred body is decoded out of freed memory -- silently,
    // and long after the call that would be blamed for it.
    //
    // This is not hypothetical: `addLibraryReference` passed a *copy* of the caller's bytes
    // as the blob while parsing from the caller's own pointer, which was harmless while
    // everything was materialized eagerly and a use-after-free once it was not. Checked
    // rather than trusted, because the cost is one range comparison per module load and the
    // failure it prevents is unattributable.
    if (onDemandIRLoad)
    {
        const Byte* const blobBegin =
            (const Byte*)decoder->blobHoldingSerializedData->getBufferPointer();
        const Byte* const blobEnd = blobBegin + decoder->blobHoldingSerializedData->getBufferSize();

        // Checks the spans themselves rather than the chunk, because these are the exact
        // pointers a deferred body dereferences later. An owned array is not a view and
        // depends on nothing, so it is skipped.
        auto spanIsInsideBlob = [&](const Byte* data, Count sizeInBytes)
        { return data >= blobBegin && data + sizeInBytes <= blobEnd; };

        // Every view-capable array, not a sample of them. Which ones actually end up as
        // views depends on the backend and on what the module contains -- one with no
        // string constants leaves `stringChars` empty and owned -- so checking a subset
        // would pass whenever the arrays it happened to pick were the owned ones.
        auto arrayIsSafe = [&](auto const& array, size_t elementSize)
        {
            return !array.isView() || spanIsInsideBlob(
                                          (const Byte*)array.getBuffer(),
                                          array.getCount() * Count(elementSize));
        };
        const bool spansAreOwnedByTheBlob = arrayIsSafe(flat.childCounts, sizeof(Int64)) &&
                                            arrayIsSafe(flat.operandIndices, sizeof(Int64)) &&
                                            arrayIsSafe(flat.stringLengths, sizeof(Int64)) &&
                                            arrayIsSafe(flat.stringChars, sizeof(uint8_t)) &&
                                            arrayIsSafe(flat.literals, sizeof(UInt64));

        if (!spansAreOwnedByTheBlob)
        {
            _noteDeferralDeclinedForSpanMismatch();
            // Fall back to an eager load rather than asserting. A caller that supplies an
            // unrelated blob then gets correct behaviour at the old cost, which is a better
            // failure mode than aborting a compile -- and eager loading is exactly what this
            // path did before deferral existed.
            onDemandIRLoad = false;
        }
    }
    /// Per-instruction predicate: is instruction `i` part of the eager *skeleton*?
    ///
    /// The skeleton is the set of instructions materialized at load time -- the module
    /// inst, its globals, and each global's decorations including anything nested under
    /// them. Everything else is a deferrable body. This is the one name for that set;
    /// "on-demand load" names the mode that produces it.
    ///
    /// A predicate rather than a command, so `!instIsEager[i]` reads as "instruction `i`
    /// was deferred" at the use site below.
    List<uint8_t> instIsEager;
    if (onDemandIRLoad)
    {
        instIsEager.setCount(numInsts);
        ::memset(instIsEager.getBuffer(), 0, size_t(numInsts));

        // Preorder scan tracking depth, allocating nothing. `childCounts` is in the
        // same preorder as the instructions, so a stack of remaining-child counts
        // is enough to recover each instruction's depth.
        //
        // This decides the same cut that `decodeInst` decides again while walking:
        // what is eager skeleton and what is a deferrable body. The two must agree
        // exactly -- an instruction this scan leaves unallocated but the decoder does
        // not defer would be wired against an empty slot, and the reverse would defer
        // a body whose slots were never filled. They agree because both derive from
        // one rule, that a global's decorations are eager and everything after them is
        // body, but they express it differently and there is no single predicate
        // enforcing it. Sharing one is the obvious follow-up; `readInstRef` asserting
        // that no live operand resolves to an empty slot is what would catch a
        // disagreement today.
        List<Int64> remainingChildren;
        Int64 depth = 0;
        // True while the scan is inside a global's decoration, including anything
        // nested under it. A decoration is kept eager because the symbol index reads
        // it without materializing, so its children have to be kept too: they are
        // reachable only through the decoration, and nothing on that path would ever
        // trigger a materialization to supply them. Keeping just the decoration inst
        // would silently give a decoration-with-children no children under on-demand load.
        bool inDecorationSubtree = false;
        for (Int64 i = 0; i < numInsts; ++i)
        {
            const IROp op = flat.instAllocInfo[i].op;
            const bool isDecoration = op >= kIROp_FirstDecoration && op <= kIROp_LastDecoration;
            // Depth 0 is the module inst and depth 1 its globals; a global's
            // decorations sit at depth 2 and carry the linkage names.
            if (depth <= kBodyChildDepth)
                inDecorationSubtree = false;
            if (depth == kBodyChildDepth && isDecoration)
                inDecorationSubtree = true;
            instIsEager[i] = uint8_t(depth <= kGlobalValueDepth || inDecorationSubtree);

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

    for (Int64 instIndex = 0; instIndex < numInsts; ++instIndex)
    {
        const auto& a = flat.instAllocInfo[instIndex];
        IROp op = a.op;
        if (op == kIROp_Invalid) [[unlikely]]
        {
            readContext._foundUnrecognizedInstructions = true;
            op = kIROp_Unrecognized;
        }
        const size_t minSizeInBytes = _readInstMinSizeInBytes(op, flat, allocStringLengthCursor);
        // Under on-demand load the skipped instructions are never allocated; the
        // preorder walk below still consumes their operand and payload cursors so
        // that positions stay correct for the instructions that are kept.
        insts[instIndex] = (onDemandIRLoad && !instIsEager[instIndex])
                               ? nullptr
                               : module->_allocateInst(op, a.operandCount, minSizeInBytes);
    }

    decoder->deferBodies = onDemandIRLoad;
    const auto moduleInst = decoder->decodeInst(nullptr, kModuleInstDepth);

    // Keep the decoder alive so the bodies it skipped can still be decoded. It
    // holds the flat table and the instruction array, which a body needs: its
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
    // Propagate what the decode walk saw, while the context is still alive.
    readContext._foundUnrecognizedInstructions |= decoder->foundUnrecognizedInstructions;

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

static Index _countChildrenOf(IRInst* inst)
{
    Index count = 0;
    for (IRInst* child : inst->getChildren())
    {
        SLANG_UNUSED(child);
        count++;
    }
    return count;
}

void _testRoundTripDecorationWithChildren(
    slang::IGlobalSession* globalSession,
    Index& outExpectedChildren,
    Index& outActualChildren,
    bool& outBodyWasDeferred)
{
    outExpectedChildren = 0;
    outActualChildren = 0;
    outBodyWasDeferred = false;

    Session* session = static_cast<Session*>(globalSession);

    // Build a function whose decoration is itself a parent, and which also has a body.
    //
    // The shape is the point, not what the instructions mean. A global value's children
    // are its decorations followed by its body, and deferral cuts between the two -- so a
    // decoration with children of its own puts instructions on the eager side of that cut
    // at the same depth as instructions on the deferred side.
    // `DifferentiableTypeDictionaryDecoration` is used only because it is a decoration
    // declared `parent = true`; nothing here depends on autodiff.
    RefPtr<IRModule> original = IRModule::create(session);
    IRInst* originalFunc = nullptr;
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        originalFunc = builder.createFunc();

        IRInst* dict = builder.addDifferentiableTypeDictionaryDecoration(originalFunc);
        IRInst* floatType = builder.getFloatType();
        builder.addDifferentiableTypeEntry(dict, floatType, floatType);
        builder.addDifferentiableTypeEntry(dict, floatType, floatType);

        // Something to defer. With only decorations, nothing is deferred and the round
        // trip would say nothing about the cut.
        builder.setInsertInto(originalFunc);
        builder.emitBlock();
        builder.emitReturn();
    }

    if (IRInst* dict =
            originalFunc->findDecorationImpl(kIROp_DifferentiableTypeDictionaryDecoration))
    {
        outExpectedChildren = _countChildrenOf(dict);
    }

    OwnedMemoryStream stream(FileAccess::ReadWrite);
    {
        RIFF::Builder riffBuilder;
        RIFF::BuildCursor cursor(riffBuilder);
        // The IR chunk is written as the root, so it can be found again without pulling in
        // the surrounding module-container layout, which this has no use for.
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, PropertyKeys<IRModule>::IRModule);
        writeSerializedModuleIR(cursor, original, nullptr);
        if (SLANG_FAILED(riffBuilder.writeTo(&stream)))
            return;
    }

    // Read back out of a blob, which is what makes deferral possible: the flat table holds
    // spans into these bytes rather than copies, so a body decoded later needs them still
    // alive. `readSerializedModuleIR` loads eagerly when handed null.
    const auto contents = stream.getContents();
    List<uint8_t> bytes;
    bytes.addRange(contents.getBuffer(), contents.getCount());
    ComPtr<ISlangBlob> blob = ListBlob::create(bytes);

    auto rootChunk = RIFF::RootChunk::getFromBlob(blob->getBufferPointer(), blob->getBufferSize());
    if (!rootChunk)
        return;

    // The root here is the `ir  ` list chunk written above, and the module is its first
    // child -- the same step `ModuleChunk::findIR()` takes. Handing the list chunk itself
    // to the reader instead walks the wrong level and corrupts the heap.
    auto irChunk = rootChunk->getFirstChild().get();
    if (!irChunk)
        return;

    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(readSerializedModuleIR(irChunk, session, nullptr, blob, reloaded)))
        return;

    IRInst* func = nullptr;
    for (IRInst* child : reloaded->getModuleInst()->getChildren())
    {
        if (child->getOp() == kIROp_Func)
        {
            func = child;
            break;
        }
    }
    if (!func)
        return;

    outBodyWasDeferred = func->m_hasDeferredBody;

    // Walking the decoration list does not materialize the body -- that is the access
    // pattern decorations are kept eager to serve, and the one this rule protects.
    if (IRInst* dict = func->findDecorationImpl(kIROp_DifferentiableTypeDictionaryDecoration))
    {
        outActualChildren = _countChildrenOf(dict);
    }
}


/// Serializes `module` and reads it back out of a blob, which is the condition that lets
/// bodies stay encoded. Shared by the two test hooks below.
/// How `_testRoundTrip` should hand the serialized bytes to the reader.
enum class TestBlobMode
{
    /// The blob the bytes were parsed out of. The only shape that permits deferral.
    Matching,
    /// No blob at all, which is what a caller reading from its own buffer supplies.
    Null,
    /// A blob holding an identical *copy* at a different address -- the shape that
    /// `addLibraryReference` had, and the one the containment check exists to catch.
    Mismatched,
};

static SlangResult _testRoundTrip(
    IRModule* module,
    Session* session,
    ComPtr<ISlangBlob>& outBlob,
    RefPtr<IRModule>& outModule,
    TestBlobMode blobMode = TestBlobMode::Matching)
{
    OwnedMemoryStream stream(FileAccess::ReadWrite);
    {
        RIFF::Builder riffBuilder;
        RIFF::BuildCursor cursor(riffBuilder);
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, PropertyKeys<IRModule>::IRModule);
        writeSerializedModuleIR(cursor, module, nullptr);
        SLANG_RETURN_ON_FAIL(riffBuilder.writeTo(&stream));
    }

    const auto contents = stream.getContents();
    List<uint8_t> bytes;
    bytes.addRange(contents.getBuffer(), contents.getCount());
    outBlob = ListBlob::create(bytes);

    auto rootChunk =
        RIFF::RootChunk::getFromBlob(outBlob->getBufferPointer(), outBlob->getBufferSize());
    if (!rootChunk)
        return SLANG_FAIL;
    // The root is the `ir  ` list chunk written above and the module is its first child --
    // the step `ModuleChunk::findIR()` takes. Handing the list chunk itself to the reader
    // walks the wrong level and corrupts the heap.
    auto irChunk = rootChunk->getFirstChild().get();
    if (!irChunk)
        return SLANG_FAIL;

    ISlangBlob* blobForReader = outBlob;
    ComPtr<ISlangBlob> decoyBlob;
    switch (blobMode)
    {
    case TestBlobMode::Null:
        blobForReader = nullptr;
        break;
    case TestBlobMode::Mismatched:
        // Same bytes, different allocation. Deferral must decline: the chunk pointers and
        // spans refer into `outBlob`, so retaining this one would keep the wrong memory
        // alive and leave the views dangling the moment `outBlob` went away.
        decoyBlob = ListBlob::create(bytes);
        blobForReader = decoyBlob;
        break;
    case TestBlobMode::Matching:
        break;
    }

    return readSerializedModuleIR(irChunk, session, nullptr, blobForReader, outModule);
}

void _testConcurrentBodyMaterialization(
    slang::IGlobalSession* globalSession,
    Index& outDeferredCount,
    Index& outMismatches)
{
    outDeferredCount = 0;
    outMismatches = 0;

    Session* session = static_cast<Session*>(globalSession);

    // Enough functions that the threads spread across bodies rather than all queuing on
    // one, and enough instructions per body that a partially published chain is visible as
    // a short one rather than needing exact timing to catch.
    static const Index kFuncCount = 64;
    static const Index kBodyInstCount = 24;

    RefPtr<IRModule> original = IRModule::create(session);
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        for (Index f = 0; f < kFuncCount; ++f)
        {
            IRInst* func = builder.createFunc();
            // Decorations are required for this to test what it claims. A deferred body is
            // published into the link *after the last decoration*, so with none the body
            // attaches at `first`, the decoration walk starts at null and ends immediately,
            // and the acquire on that link is never exercised.
            builder.addNameHintDecoration(func, UnownedStringSlice("concurrentProbe"));
            builder.setInsertInto(func);
            builder.emitBlock();
            for (Index i = 0; i < kBodyInstCount - 2; ++i)
            {
                IRType* floatType = builder.getFloatType();
                builder.emitAdd(
                    floatType,
                    builder.getFloatValue(floatType, IRFloatingPointValue(i)),
                    builder.getFloatValue(floatType, IRFloatingPointValue(1)));
            }
            builder.emitReturn();
            builder.setInsertInto(original->getModuleInst());
        }
    }

    ComPtr<ISlangBlob> blob;
    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(_testRoundTrip(original, session, blob, reloaded)))
        return;

    List<IRInst*> funcs;
    for (IRInst* child : reloaded->getModuleInst()->getChildren())
    {
        if (child->getOp() == kIROp_Func)
            funcs.add(child);
    }
    if (funcs.getCount() != kFuncCount)
        return;

    for (IRInst* func : funcs)
    {
        if (func->m_hasDeferredBody)
            outDeferredCount++;
    }

    // Counted from the pre-serialization module, so the expectation does not come from the
    // path under test.
    List<Index> expected;
    for (IRInst* child : original->getModuleInst()->getChildren())
    {
        if (child->getOp() != kIROp_Func)
            continue;
        expected.add(_countChildrenOf(child));
    }
    if (expected.getCount() != funcs.getCount())
        return;

    // Released together so every thread arrives at the same untouched body at once. Staggered
    // starts would let each body finish materializing before the next thread reached it,
    // which is the uncontended case the other tests already cover.
    static const int kThreadCount = 8;
    std::atomic<bool> go{false};
    std::atomic<Index> mismatches{0};
    List<std::thread> threads;
    for (int t = 0; t < kThreadCount; ++t)
    {
        threads.add(std::thread(
            [&, threadIndex = t]()
            {
                while (!go.load(std::memory_order_acquire))
                    std::this_thread::yield();
                // Half the threads publish, half walk decorations. Materializing from
                // every thread exercises the mutex but not the barrier that matters most:
                // the decoration walk is the one reader allowed to observe the publication
                // link *without* going through `ensureBodyMaterialized`, which is why
                // `getFirstDecoration`, `getNextDecoration` and
                // `IRDecorationList::Iterator::operator++` load it with acquire. Unless
                // some thread is walking decorations while another publishes into
                // `lastDecoration->next`, that race is never run and dropping those
                // acquires passes every test.
                const bool walksDecorations = (threadIndex % 2) == 1;
                for (Index i = 0; i < funcs.getCount(); ++i)
                {
                    if (walksDecorations)
                    {
                        // Must never run past the decorations into a body that another
                        // thread is publishing. Counting is enough to catch it: a walk
                        // that continues into the body returns more than there are
                        // decorations.
                        Index decorationCount = 0;
                        for (IRDecoration* decoration : funcs[i]->getDecorations())
                        {
                            SLANG_UNUSED(decoration);
                            decorationCount++;
                        }
                        // Exactly the one decoration added above. More than that means the
                        // walk followed a link into a body another thread was publishing
                        // and kept going, counting body instructions as decorations.
                        if (decorationCount != 1)
                            mismatches.fetch_add(1);
                    }
                    else
                    {
                        // The first touch of each body: this is what takes the loader's
                        // mutex and, on the winning thread, publishes the chain with a
                        // release store.
                        funcs[i]->ensureBodyMaterialized();
                        if (_countChildrenOf(funcs[i]) != expected[i])
                            mismatches.fetch_add(1);
                    }
                }
            }));
    }
    go.store(true, std::memory_order_release);
    for (auto& thread : threads)
        thread.join();

    outMismatches = mismatches.load();
}


void _testDeferralFallback(
    slang::IGlobalSession* globalSession,
    int blobMode,
    bool& outDeferredLoaderInstalled,
    Index& outInstCount,
    Index& outSpanMismatchDelta)
{
    outDeferredLoaderInstalled = false;
    outInstCount = 0;
    outSpanMismatchDelta = 0;

    Session* session = static_cast<Session*>(globalSession);

    // A module with several bodies, so a deferred load has something to defer and an
    // eager one has something to get wrong.
    RefPtr<IRModule> original = IRModule::create(session);
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        for (Index f = 0; f < 8; ++f)
        {
            IRInst* func = builder.createFunc();
            builder.setInsertInto(func);
            builder.emitBlock();
            IRType* floatType = builder.getFloatType();
            for (Index i = 0; i < 6; ++i)
            {
                builder.emitAdd(
                    floatType,
                    builder.getFloatValue(floatType, IRFloatingPointValue(i)),
                    builder.getFloatValue(floatType, IRFloatingPointValue(1)));
            }
            builder.emitReturn();
            builder.setInsertInto(original->getModuleInst());
        }
    }

    const Index mismatchBefore = getDeferralDeclinedForSpanMismatchCount();

    ComPtr<ISlangBlob> blob;
    RefPtr<IRModule> reloaded;
    const TestBlobMode mode = TestBlobMode(blobMode);
    if (SLANG_FAILED(_testRoundTrip(original, session, blob, reloaded, mode)))
        return;

    outSpanMismatchDelta = getDeferralDeclinedForSpanMismatchCount() - mismatchBefore;
    outDeferredLoaderInstalled = (reloaded->getDeferredBodyLoader() != nullptr);

    // Counting every instruction forces every body to materialize if it was deferred, and
    // reads every body if it was not -- so the same number must come back either way. That
    // is the property the fallbacks exist to preserve: declining deferral may cost time,
    // but it must never change what was loaded.
    Index count = 0;
    for (IRInst* global : reloaded->getModuleInst()->getChildren())
    {
        count++;
        for (IRInst* child : global->getChildren())
        {
            count++;
            for (IRInst* grandchild : child->getChildren())
            {
                SLANG_UNUSED(grandchild);
                count++;
            }
        }
    }
    outInstCount = count;
}

} // namespace Slang
