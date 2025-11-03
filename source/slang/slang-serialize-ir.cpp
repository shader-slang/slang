// slang-serialize-ir.cpp
#include "slang-serialize-ir.h"

#include "core/slang-blob-builder.h"
#include "core/slang-common.h"
#include "core/slang-dictionary.h"
#include "core/slang-performance-profiler.h"
#include "slang-ir-insts-stable-names.h"
#include "slang-ir-insts.h"
#include "slang-ir-validate.h"
#include "slang-serialize-fossil.h"
#include "slang-serialize-riff.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"
#include "slang-tag-version.h"
#include "slang.h"

//
#include "slang-serialize-ir.cpp.fiddle"

// If USE_RIFF is set, then we serialize using the RIFF backend, it's the
// slowest option
#define USE_RIFF 0
// If we are serializing using Fossil, DIRECT_FROM_FOSSIL will make it so that
// we unflatten directly from the fossilized representation rather than
// deserializing everything first. It is the fastest option
#define DIRECT_FROM_FOSSIL 0

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
    FIDDLE() List<Int64> childCounts;
    FIDDLE() List<SourceLoc> sourceLocs;

    // The length of operandIndices is the number of instructions in the module
    // (for typeUse) + the number of operands in the module
    //
    // a nullptr operand is encoded as -1
    FIDDLE() List<Int64> operandIndices;

    // The length is equal to the number of strings and blobs in the module
    FIDDLE() List<Int64> stringLengths;

    // The length is the sum of all stringLengths, the contents is the
    // concatenation of all their data
    FIDDLE() List<uint8_t> stringChars;

    // The length is number of integer/floating constants in the module, and
    // the contents are the bits of those constants
    FIDDLE() List<UInt64> literals;
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
#if USE_RIFF
using IRWriteSerializer = Serializer<RIFFSerialWriter, IRSerialWriteContext>;
using IRReadSerializer = Serializer<RIFFSerialReader, IRSerialReadContext>;
#else
using IRWriteSerializer = Serializer<Fossil::SerialWriter, IRSerialWriteContext>;
using IRReadSerializer = Serializer<Fossil::SerialReader, IRSerialReadContext>;
#endif

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
                flat.childCounts[inst->parent->scratchData]++;
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

// A helper function to read one thing from a Fossil ref
template<typename T>
static T deserialize1(const IRReadSerializer& serializer, const Fossil::AnyValRef r)
{
    T t;
    Fossil::ReadContext context;
    Fossil::SerialReader reader(
        context,
        Fossil::getAddress(r),
        Fossil::SerialReader::InitialStateType::PseudoPtr);
    IRReadSerializer serializer_(&reader, serializer.getContext());
    serialize(serializer_, t);
    return t;
}

static IRModuleInst* deserializeFromFlatModule(const IRReadSerializer& serializer, IRModule* module)
{
    IRSerialReadContext& readContext = *serializer.getContext();
#if DIRECT_FROM_FOSSIL
    const auto flatPtr = as<Fossilized<FlatInstTable>>(serializer.getImpl()->readValPtr());
    Fossilized<FlatInstTable>& flat = flatPtr->getDataRef();
    // Read just the sourceLocs using normal deserialization
    const auto sourceLocs = deserialize1<List<SourceLoc>>(serializer, flatPtr->getSourceLocs());
#else
    FlatInstTable flat;
    serialize(serializer, flat);
    const List<SourceLoc>& sourceLocs = flat.sourceLocs;
    // dumpFlatInstTableStats(flat, "deserializing");
#endif

    Int64 stringLengthIndex = 0;
    List<IRInst*> instsList;

#if DIRECT_FROM_FOSSIL
    const auto numInsts = flat.instAllocInfo.getElementCount();
#else
    const auto numInsts = flat.instAllocInfo.getCount();
#endif

    instsList.setCount(numInsts + 1);
    // nullptr instructions are represented as `-1`. We can save ourselves a
    // branch by just making that index valid.
    IRInst** const insts = &instsList[1];
    insts[-1] = nullptr;

    for (Int64 instIndex = 0; instIndex < numInsts; ++instIndex)
    {
        const auto& a = flat.instAllocInfo[instIndex];
        // The opcode is serialized as the stable name, so if we're reading
        // directly we need to destabilize that
#if DIRECT_FROM_FOSSIL
        IROp op = getStableNameOpcode(a.op);
#else
        IROp op = a.op;
#endif
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
            minSizeInBytes = offsetof(IRConstant, value) +
                             offsetof(IRConstant::StringValue, chars) +
                             flat.stringLengths[stringLengthIndex++];
            break;
        }
        insts[instIndex] = module->_allocateInst(op, a.operandCount, minSizeInBytes);
    }

    Int64 litIndex = 0;
    Int64 operandIndex = 0;
    Int64 instIndex = 0;
    stringLengthIndex = 0;
    Int64 stringDataIndex = 0;
    const auto go = [&](auto& go, IRInst* parent) -> IRInst*
    {
        const auto thisInstIndex = instIndex++;
        IRInst* inst = insts[thisInstIndex];

        // operands and sourcelocs
        inst->sourceLoc = sourceLocs[thisInstIndex];
        inst->typeUse.init(inst, insts[flat.operandIndices[operandIndex++]]);
        for (Int64 o = 0; o < inst->operandCount; ++o)
            inst->getOperands()[o].init(inst, insts[flat.operandIndices[operandIndex++]]);

        // Handle special instructions
        switch (inst->m_op)
        {
        [[unlikely]] case kIROp_ModuleInst:
            cast<IRModuleInst>(inst)->module = module;
            break;
        case kIROp_BoolLit:
        case kIROp_IntLit:
            cast<IRConstant>(inst)->value.intVal =
                bitCast<IRIntegerValue>(flat.literals[litIndex++]);
            break;
        case kIROp_FloatLit:
            cast<IRConstant>(inst)->value.floatVal = bitCast<double>(flat.literals[litIndex++]);
            break;
        case kIROp_PtrLit:
            // Keep the compiler happy on 32 bit builds
            cast<IRConstant>(inst)->value.ptrVal = (void*)(uintptr_t(flat.literals[litIndex++]));
            break;
        case kIROp_StringLit:
        case kIROp_BlobLit:
            const auto c = cast<IRConstant>(inst);
            const auto len = flat.stringLengths[stringLengthIndex++];
            char* const dstChars = c->value.stringVal.chars;
            c->value.stringVal.numChars = uint32_t(len);
            memcpy(dstChars, flat.stringChars.begin() + stringDataIndex, len);
            stringDataIndex += len;
            break;
        }

        // Read in children, and fix up pointers
        inst->parent = parent;
        IRInst* prev = nullptr;
        IRInst* first = nullptr;
        IRInst* last = nullptr;
        for (Int64 i = 0; i < flat.childCounts[thisInstIndex]; ++i)
        {
            auto c = go(go, inst);
            if (i == 0)
                first = c;
            last = c;
            c->prev = prev;
            if (prev)
                prev->next = c;
            prev = c;
        }
        if (last)
            last->next = nullptr;
        inst->m_decorationsAndChildren.first = first;
        inst->m_decorationsAndChildren.last = last;

        return inst;
    };
    const auto moduleInst = go(go, nullptr);
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

#if USE_RIFF
    {
        RIFFSerialWriter writer(cursor.getCurrentChunk());
        IRSerialWriteContext context{sourceLocWriter};
        IRWriteSerializer serializer(&writer, &context);
        serialize(serializer, moduleInfo);
    }

    ComPtr<ISlangBlob> blob;
#else
    BlobBuilder blobBuilder;
    {
        Fossil::SerialWriter writer(blobBuilder);
        IRSerialWriteContext context{sourceLocWriter};
        IRWriteSerializer serializer(&writer, &context);
        serialize(serializer, moduleInfo);
    }

    ComPtr<ISlangBlob> blob;
    blobBuilder.writeToBlob(blob.writeRef());

    void const* data = blob->getBufferPointer();
    size_t size = blob->getBufferSize();
    cursor.addDataChunk(PropertyKeys<IRModule>::IRModule, data, size);
#endif
}

Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String& compilerVersion,
    UInt& version,
    String& name)
{
    static_assert(!USE_RIFF); // unimplemented

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
#if USE_RIFF
    auto dataChunk = as<RIFF::ListChunk>(chunk);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format for serialized module IR");
    }

    IRModuleInfo info;
    auto sharedDecodingContext = RefPtr(new IRSerialReadContext(session, sourceLocReader));
    {
        RIFFSerialReader reader(dataChunk);

        IRReadSerializer serializer(&reader, sharedDecodingContext);
        serialize(serializer, info);
    }
#else
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
#endif
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
