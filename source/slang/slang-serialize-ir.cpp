// slang-serialize-ir.cpp
#include "slang-serialize-ir.h"

#include "core/slang-blob-builder.h"
#include "core/slang-common.h"
#include "core/slang-dictionary.h"
#include "core/slang-performance-profiler.h"
#include "core/slang-riff.h"
#include "slang-ir-insts-stable-names.h"
#include "slang-ir-insts.h"
#include "slang-ir-validate.h"
#include "slang-ondemand-ir-stats.h"
#include "slang-serialize-fossil.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"
#include "slang-tag-version.h"
#include "slang.h"

#include <chrono>
#include <mutex>

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
    IRSerialReadContext(Session* session, SerialSourceLocReader* sourceLocReader)
        : _session(session), _sourceLocReader(sourceLocReader)
    {
    }
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
// Decoding state for a module's flat instruction table.
//
// The same walk serves two purposes, which is why it lives in an object rather
// than a lambda: it runs once over the whole module at load time, and then again
// over a single subtree each time a deferred body is asked for. Holding the flat
// table and the instruction array keeps the second use possible -- a body's
// operands are indices into that array, and may name any module-scope global.
//
struct FlatModuleDecoder : IRDeferredBodyLoader
{
    FlatInstTable flat;
    List<IRInst*> instsList; ///< index -1 is the null slot, hence `insts()`
    IRModule* module = nullptr;

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
    bool deferBodies = false;

    /// Serialises deferred decoding.
    ///
    /// A global session is shared across threads, so two compiles can reach the
    /// same builtin module at once. The decode mutates state that is global to the
    /// module -- the cursors, the instruction array and the module's arena -- so it
    /// is serialised wholesale rather than per instruction. Contention is limited
    /// to the first touch of each body.
    std::mutex mutex;

    Int64 instIndex = 0;
    Int64 operandIndex = 0;
    Int64 literalIndex = 0;
    Int64 stringLengthIndex = 0;
    Int64 stringDataIndex = 0;

    IRInst** insts() { return &instsList[1]; }
    Int64 getInstCount() const { return flat.instAllocInfo.getCount(); }

    IRInst* readInstRef()
    {
        SLANG_RELEASE_ASSERT(operandIndex < flat.operandIndices.getCount());
        const auto index = flat.operandIndices[operandIndex++];
        SLANG_RELEASE_ASSERT(index >= -1 && index < getInstCount());
        return insts()[index];
    }

    /// Decodes the instruction at the cursor and, recursively, its children.
    ///
    /// A null return means the instruction was deliberately not materialized; its
    /// operand and payload entries are still consumed so that the cursors stay
    /// aligned for the instructions that are kept.
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
    deferredBodies.remove(inst);

    // Replay this subtree from where the load walk left off, with deferral
    // disabled so nested instructions materialize in full.
    // Allocate every instruction in the subtree before wiring any of them, exactly
    // as the load-time path does. Instructions forward-reference each other --
    // a branch names a block defined later -- so an operand read before its target
    // exists would silently resolve to null.
    {
        Int64 stringLengthCursor = body.stringLengthCursor;
        const Int64 end = body.firstChildInstIndex + body.instCount;
        for (Int64 i = body.firstChildInstIndex; i < end; ++i)
        {
            if (!insts()[i])
                insts()[i] = allocateInstAt(i, stringLengthCursor);
        }
    }

    const bool savedDefer = deferBodies;
    deferBodies = false;
    instIndex = body.firstChildInstIndex;
    operandIndex = body.operandCursor;
    literalIndex = body.literalCursor;
    stringLengthIndex = body.stringLengthCursor;
    stringDataIndex = body.stringDataCursor;

    // The decorations are already linked; the body appends after them.
    IRInst* prev = inst->m_decorationsAndChildren.last;
    IRInst* first = inst->m_decorationsAndChildren.first;
    for (Int64 i = 0; i < body.childCount; ++i)
    {
        auto child = decodeInst(inst, 2);
        if (!child)
            continue;
        if (!first)
            first = child;
        child->prev = prev;
        if (prev)
            prev->next = child;
        prev = child;
    }
    if (prev)
        prev->next = nullptr;
    inst->m_decorationsAndChildren.first = first;
    inst->m_decorationsAndChildren.last = prev;

    deferBodies = savedDefer;

    // Release: a thread that later observes this as false must also see every
    // write above, so that it reads a fully linked body.
    inst->m_hasDeferredBody.store(false, std::memory_order_release);
}


/// Allocates the instruction for `instIndex`, mirroring the sizing rules of the
/// load-time allocation pass.
///
/// Needed because a deferred body's instructions were never allocated: the load
/// pass left their slots empty. String and blob constants carry their characters
/// inline, so their size depends on a length that is read from the payload stream;
/// the cursor is positioned at that length here, and peeking does not disturb it
/// because the payload switch consumes it immediately afterwards.
IRInst* FlatModuleDecoder::allocateInstAt(Int64 instIndexToAlloc, Int64& stringLengthCursor)
{
    const auto& allocInfo = flat.instAllocInfo[instIndexToAlloc];
    IROp op = allocInfo.op;
    if (op == kIROp_Invalid) [[unlikely]]
        op = kIROp_Unrecognized;

    size_t minSizeInBytes = 0;
    switch (op)
    {
    [[unlikely]] case kIROp_ModuleInst:
        minSizeInBytes = offsetof(IRModuleInst, module) + sizeof(IRModuleInst::module);
        break;
    case kIROp_BoolLit:
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_PtrLit:
    case kIROp_VoidLit:
        minSizeInBytes = offsetof(IRConstant, value) + sizeof(IRConstant::value);
        break;
    case kIROp_StringLit:
    case kIROp_BlobLit:
        {
            SLANG_RELEASE_ASSERT(stringLengthCursor < flat.stringLengths.getCount());
            const auto len = flat.stringLengths[stringLengthCursor++];
            SLANG_RELEASE_ASSERT(len >= 0);
            const size_t headerSize =
                offsetof(IRConstant, value) + offsetof(IRConstant::StringValue, chars);
            minSizeInBytes = headerSize + size_t(len);
            break;
        }
    }
    return module->_allocateInst(op, allocInfo.operandCount, minSizeInBytes);
}

IRInst* FlatModuleDecoder::decodeInst(IRInst* parent, Int64 depth)
{
    SLANG_RELEASE_ASSERT(depth < kMaxIRSerializationDepth);
    SLANG_RELEASE_ASSERT(instIndex < getInstCount());

    const auto thisInstIndex = instIndex++;
    IRInst* inst = insts()[thisInstIndex];

    // Under lazy load this instruction may have been skipped. Its operand and
    // payload entries still have to be consumed so the cursors stay aligned for
    // the instructions that were kept.
    const auto& allocInfo = flat.instAllocInfo[thisInstIndex];
    const Int64 thisOperandCount = inst ? Int64(inst->operandCount) : Int64(allocInfo.operandCount);

    // operands and sourcelocs
    if (inst)
    {
        inst->sourceLoc = flat.sourceLocs[thisInstIndex];
        inst->typeUse.init(inst, readInstRef());
        for (Int64 o = 0; o < thisOperandCount; ++o)
            inst->getOperands()[o].init(inst, readInstRef());
    }
    else
    {
        readInstRef(); // type use
        for (Int64 o = 0; o < thisOperandCount; ++o)
            readInstRef();
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
            SLANG_RELEASE_ASSERT(literalIndex < flat.literals.getCount());
            const auto bits = flat.literals[literalIndex++];
            if (inst)
                cast<IRConstant>(inst)->value.intVal = bitCast<IRIntegerValue>(bits);
            break;
        }
    case kIROp_FloatLit:
        {
            SLANG_RELEASE_ASSERT(literalIndex < flat.literals.getCount());
            const auto bits = flat.literals[literalIndex++];
            if (inst)
                cast<IRConstant>(inst)->value.floatVal = bitCast<double>(bits);
            break;
        }
    case kIROp_PtrLit:
        {
            SLANG_RELEASE_ASSERT(literalIndex < flat.literals.getCount());
            const auto bits = flat.literals[literalIndex++];
            // Keep the compiler happy on 32 bit builds
            if (inst)
                cast<IRConstant>(inst)->value.ptrVal = (void*)(uintptr_t(bits));
            break;
        }
    case kIROp_StringLit:
    case kIROp_BlobLit:
        {
            auto* const c = inst ? cast<IRConstant>(inst) : nullptr;
            SLANG_RELEASE_ASSERT(stringLengthIndex < flat.stringLengths.getCount());
            const auto len = flat.stringLengths[stringLengthIndex++];
            SLANG_RELEASE_ASSERT(len >= 0);
            SLANG_RELEASE_ASSERT(uint64_t(len) <= uint64_t(UINT32_MAX));

            const auto stringCharsCount = flat.stringChars.getCount();
            SLANG_RELEASE_ASSERT(stringDataIndex <= stringCharsCount);
            SLANG_RELEASE_ASSERT(len <= stringCharsCount - stringDataIndex);

            if (c)
            {
                char* const dstChars = c->value.stringVal.chars;
                c->value.stringVal.numChars = uint32_t(len);
                if (len != 0)
                    memcpy(dstChars, flat.stringChars.getBuffer() + stringDataIndex, size_t(len));
            }
            stringDataIndex += len;
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
        if (deferBodies && depth == 1 && inst && !inst->m_hasDeferredBody)
        {
            const IROp nextOp = flat.instAllocInfo[instIndex].op;
            const bool nextIsDecoration =
                nextOp >= kIROp_FirstDecoration && nextOp <= kIROp_LastDecoration;
            if (!nextIsDecoration)
            {
                DeferredBody body;
                body.firstChildInstIndex = instIndex;
                body.childCount = childCount - i;
                body.operandCursor = operandIndex;
                body.literalCursor = literalIndex;
                body.stringLengthCursor = stringLengthIndex;
                body.stringDataCursor = stringDataIndex;
                deferredBodies.add(inst, body);
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
        if (auto recorded = deferredBodies.tryGetValue(inst))
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
    const bool statsOn = OnDemandStats::isEnabled();
    const auto tCopyStart = std::chrono::steady_clock::now();
    const uint64_t rssCopyStart = statsOn ? OnDemandStats::getCurrentRSSBytes() : 0;
    serialize(serializer, flat);
    const auto tCopyEnd = std::chrono::steady_clock::now();
    const uint64_t rssCopyEnd = statsOn ? OnDemandStats::getCurrentRSSBytes() : 0;
    const List<SourceLoc>& sourceLocs = flat.sourceLocs;
    // dumpFlatInstTableStats(flat, "deserializing");

    List<IRInst*>& instsList = decoder->instsList;

    // Pass 1 walks the string lengths independently of the decoding cursors below,
    // purely to size the allocations for string and blob constants.
    Int64 stringLengthIndex = 0;

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

    // Lazy load materializes only what a symbol index needs -- the module inst,
    // each module-scope global, and each global's decorations -- and leaves each
    // global's body encoded until something asks for its children.
    //
    // This needs no change to the serialized format. The obstacle to decoding one
    // instruction on its own is that operands, literals and strings are consumed by
    // running cursors in preorder; those cursor positions are recovered here by a
    // scan over `childCounts` that allocates nothing.
    const bool lazyIRLoad = OnDemandStats::isLazyIRLoadEnabled();
    List<uint8_t> materializeInst;
    if (lazyIRLoad)
    {
        materializeInst.setCount(numInsts);
        ::memset(materializeInst.getBuffer(), 0, size_t(numInsts));

        // Preorder scan tracking depth, allocating nothing. `childCounts` is in the
        // same preorder as the instructions, so a stack of remaining-child counts
        // is enough to recover each instruction's depth.
        List<Int64> remainingChildren;
        Int64 depth = 0;
        for (Int64 i = 0; i < numInsts; ++i)
        {
            const IROp op = flat.instAllocInfo[i].op;
            const bool isDecoration = op >= kIROp_FirstDecoration && op <= kIROp_LastDecoration;
            // Depth 0 is the module inst and depth 1 its globals; a global's
            // decorations sit at depth 2 and carry the linkage names.
            materializeInst[i] = uint8_t(depth <= 1 || (depth == 2 && isDecoration));

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

        Int64 kept = 0;
        for (Int64 i = 0; i < numInsts; ++i)
            kept += materializeInst[i];
        OnDemandStats::recordLazyLoadCounts(kept, numInsts);
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
        size_t minSizeInBytes = 0;
        switch (op)
        {
        [[unlikely]] case kIROp_ModuleInst:
            minSizeInBytes = offsetof(IRModuleInst, module) +
                             sizeof(IRModuleInst::module); // NOLINT(bugprone-sizeof-expression)
            break;
        case kIROp_BoolLit:
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_PtrLit:
        case kIROp_VoidLit:
            minSizeInBytes = offsetof(IRConstant, value) + sizeof(IRConstant::value);
            break;
        // About 5% of instructions in the core module are strings!
        case kIROp_StringLit:
        case kIROp_BlobLit:
            {
                SLANG_RELEASE_ASSERT(stringLengthIndex < flat.stringLengths.getCount());
                const auto len = flat.stringLengths[stringLengthIndex++];
                SLANG_RELEASE_ASSERT(len >= 0);
                SLANG_RELEASE_ASSERT(uint64_t(len) <= uint64_t(UINT32_MAX));

                const size_t headerSize =
                    offsetof(IRConstant, value) + offsetof(IRConstant::StringValue, chars);
                SLANG_RELEASE_ASSERT(size_t(len) <= size_t(-1) - headerSize);

                minSizeInBytes = headerSize + size_t(len);
                break;
            }
        }
        // In skeleton mode the skipped instructions are never allocated; the
        // preorder walk below still consumes their operand and payload cursors so
        // that positions stay correct for the instructions that are kept.
        insts[instIndex] = (lazyIRLoad && !materializeInst[instIndex])
                               ? nullptr
                               : module->_allocateInst(op, a.operandCount, minSizeInBytes);
    }

    const auto tAllocEnd = std::chrono::steady_clock::now();
    const uint64_t rssAllocEnd = statsOn ? OnDemandStats::getCurrentRSSBytes() : 0;

    decoder->deferBodies = lazyIRLoad;
    const auto moduleInst = decoder->decodeInst(nullptr, 0);

    // Keep the decoder alive so the bodies it skipped can still be decoded. It
    // holds the flat table and the instruction array, which a body needs: its
    // operands are indices into that array and may name any module-scope global.
    if (decoder->deferredBodies.getCount())
    {
        module->setDeferredBodyLoader(decoder);
    }

    if (statsOn)
    {
        const auto tWireEnd = std::chrono::steady_clock::now();
        using Ms = std::chrono::duration<double, std::milli>;
        OnDemandStats::recordIRSubPhases(
            {Ms(tCopyEnd - tCopyStart).count(),
             Ms(tAllocEnd - tCopyEnd).count(),
             Ms(tWireEnd - tAllocEnd).count(),
             int64_t(rssCopyEnd) - int64_t(rssCopyStart),
             int64_t(rssAllocEnd) - int64_t(rssCopyEnd)});
    }

    // Record the flat-table shape while it is still in scope; the caller only
    // ever sees the materialized IRModule.
    //
    // Off unless the walk is asked for: reading each global's decorations
    // materializes deferred children, and this runs inside the window the IR
    // phase record is timing, so leaving it on makes the eager tier measure
    // larger than it is.
    if (OnDemandStats::isWalkEnabled())
    {
        OnDemandStats::IRModuleShape shape;
        shape.instCount = numInsts;
        shape.globalInstCount = 0;
        shape.eagerTierInstCount = 1; // the module inst itself
        for (auto child = moduleInst->getFirstChild(); child; child = child->getNextInst())
        {
            shape.globalInstCount++;
            // Size the eager tier a per-symbol lazy design would still need: each
            // global's header, plus the linkage decoration and the string that
            // carries its mangled name, since those are what the symbol index is
            // built from.
            shape.eagerTierInstCount++;
            for (auto decoration : child->getDecorations())
            {
                if (decoration->getOp() == kIROp_ExportDecoration ||
                    decoration->getOp() == kIROp_ImportDecoration)
                {
                    shape.eagerTierInstCount += 2; // decoration + its name operand
                }
            }
        }
        shape.operandSlotCount = operandIndicesCount;
        shape.stringByteCount = flat.stringChars.getCount();
        shape.literalCount = flat.literals.getCount();
        shape.serializedByteCount = 0;
        shape.arenaBytesUsed = 0;
        OnDemandStats::recordIRModuleShape(shape);
    }

    // The walk visits every instruction and consumes every payload entry even when
    // bodies are deferred -- deferring skips materialization, not traversal -- so
    // these end-state checks hold either way.
    SLANG_RELEASE_ASSERT(decoder->instIndex == numInsts);
    SLANG_RELEASE_ASSERT(decoder->operandIndex == operandIndicesCount);
    // Unknown future opcodes intentionally become a recoverable read failure later.
    // This reader cannot know whether those opcodes consume literal or string payloads.
    if (!readContext._foundUnrecognizedInstructions)
    {
        SLANG_RELEASE_ASSERT(decoder->literalIndex == flat.literals.getCount());
        SLANG_RELEASE_ASSERT(decoder->stringLengthIndex == flat.stringLengths.getCount());
        SLANG_RELEASE_ASSERT(decoder->stringDataIndex == flat.stringChars.getCount());
    }
    // Diagnostic: materialize everything immediately. This separates "is the
    // deferred decode correct?" from "does every reader go through the hook?" --
    // with this on, the module should be indistinguishable from an eager load.
    if (::getenv("SLANG_ONDEMAND_FORCE_MATERIALIZE"))
    {
        List<IRInst*> pending;
        for (const auto& [deferredInst, unused] : decoder->deferredBodies)
        {
            SLANG_UNUSED(unused);
            pending.add(deferredInst);
        }
        for (auto deferredInst : pending)
            decoder->materializeDeferredBody(deferredInst);
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
    auto sharedDecodingContext = RefPtr(new IRSerialReadContext(session, sourceLocReader));
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
    RefPtr<IRModule>& outIRModule)
{
    SLANG_PROFILE;

    SLANG_RETURN_ON_FAIL(readSerializedModuleIR_(chunk, session, sourceLocReader, outIRModule));

    //
    // Module is finally valid (or at least as much as it was going it) and
    // ready to be used
    //
    outIRModule->buildMangledNameToGlobalInstMap();

    return SLANG_OK;
}


} // namespace Slang
