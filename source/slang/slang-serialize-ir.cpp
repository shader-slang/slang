// slang-serialize-ir.cpp
#include "slang-serialize-ir.h"

#include "core/slang-blob-builder.h"
#include "slang-ir-insts-stable-names.h"
#include "slang-ir-insts.h"
#include "slang-ir-validate.h"
#include "slang-serialize-fossil.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"
#include "slang-tag-version.h"
#include "slang.h"

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
FIDDLE()
struct IRModuleInfo
{
    FIDDLE(...)
    // Include the specific compiler version in serialized output, in case we
    // ever need to do any version specific workarounds.
    FIDDLE() String fullVersion = SLANG_TAG_VERSION;
    // Include this here so that if we need to change the way we serialize
    // things and maintain backwards compat we can increment this value, for
    // example if we introduce more instructions with weird payloads like
    // IRModuleInst or IRConstants.
    const static UInt kSupportedSerializationVersion = 0;
    FIDDLE() UInt serializationVersion = kSupportedSerializationVersion;
    FIDDLE() RefPtr<IRModule> module;
};

//
// We need some small amount of additional context to serialize IR Modules, keep track of that here
//
struct IRSerialContext;
using IRSerializer = Serializer_<ISerializerImpl, IRSerialContext>;

struct IRSerialContext : SourceLocSerialContext
{
public:
    virtual void handleIRModule(IRSerializer const& serializer, IRModule*& value) = 0;
    virtual void handleName(IRSerializer const& serializer, Name*& value) = 0;
};

struct IRSerialWriteContext : IRSerialContext
{
    IRSerialWriteContext(SerialSourceLocWriter* sourceLocWriter)
        : _sourceLocWriter(sourceLocWriter)
    {
    }

    virtual void handleIRModule(IRSerializer const& serializer, IRModule*& value) override;
    virtual void handleName(IRSerializer const& serializer, Name*& value) override;
    virtual SerialSourceLocWriter* getSourceLocWriter() override { return _sourceLocWriter; }

    SerialSourceLocWriter* _sourceLocWriter;
};

struct IRSerialReadContext : IRSerialContext, RefObject
{
    IRSerialReadContext(Session* session, SerialSourceLocReader* sourceLocReader)
        : _session(session), _sourceLocReader(sourceLocReader)
    {
    }
    virtual void handleIRModule(IRSerializer const& serializer, IRModule*& value) override;
    virtual void handleName(IRSerializer const& serializer, Name*& value) override;
    virtual SerialSourceLocReader* getSourceLocReader() override { return _sourceLocReader; }

    // Used to allocate an IRModule
    Session* _session;

    //
    SerialSourceLocReader* _sourceLocReader;

    // The module in which we will allocate our instructions
    RefPtr<IRModule> _module;
};

SLANG_DECLARE_FOSSILIZED_AS(Name, String);

/// Fossilized representation of a `IRModule`
struct Fossilized_IRModule;

SLANG_DECLARE_FOSSILIZED_TYPE(IRModule, Fossilized_IRModule);

struct Fossilized_IRModule : public FossilizedRecordVal
{
    Fossilized<decltype(IRModule::m_moduleInst)> m_moduleInst;
    Fossilized<String> m_name;
    Fossilized<decltype(IRModule::m_version)> m_version;
};

//
// This splice handles any aggregate types, a similar splice is well documented
// in slang-serialize-ast.cpp
//
#if 0 // FIDDLE TEMPLATE:
% irStructTypes = {
%   Slang.IRModuleInfo,
% }
%
% for _,T in ipairs(irStructTypes) do

/// Fossilized representation of a `$T`
struct Fossilized_$T;

SLANG_DECLARE_FOSSILIZED_TYPE($T, Fossilized_$T);

/// Serialize a `$T`
void serialize(IRSerializer const& serializer, $T& value);
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
%   for _,f in ipairs(T.directFields) do
    Fossilized<decltype($T::$f)> $f;
%   end
};

/// Serialize a `value` of type `$T`
void serialize(IRSerializer const& serializer, $T& value)
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


// IROps are serialized as integers, and given a stable name
SLANG_DECLARE_FOSSILIZED_AS(IROp, FossilUInt);
void serialize(Serializer const& serializer, IROp& value)
{
    auto stableName = isWriting(serializer) ? getOpcodeStableName(value) : kInvalidStableName;
    serializeEnum(serializer, stableName);
    if (isReading(serializer))
    {
        value = getStableNameOpcode(stableName);
        // It's possible we're reading a module serialized by a future version of
        // Slang with as-yet unknown instructions.
        // if this is the case, return IRUnrecognized and we can handle it later
        if (value == kIROp_Invalid)
            value = kIROp_Unrecognized;
    }
}

//
// Serialized linked list of child instructions as regular lists, we can fix up
// the pointers on deserialization
//
SLANG_DECLARE_FOSSILIZED_AS(IRInstListBase, List<IRInst*>);

void serialize(IRSerializer const& serializer, IRInstListBase& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);

    if (isWriting(serializer))
    {
        for (auto inst : value)
        {
            serialize(serializer, inst);
        }
    }
    else
    {
        IRInst* first = nullptr;
        IRInst* prev = nullptr;

        while (hasElements(serializer))
        {
            IRInst* inst = nullptr;
            serialize(serializer, inst);
            first = first ? first : inst;

            if (prev)
            {
                prev->next = inst;
            }

            inst->prev = prev;
            prev = inst;
        }
        if (prev)
        {
            prev->next = nullptr;
        }
        value = IRInstListBase(first, prev);
    }
}

//
// Initializing an IRUse requires a small bit of special setup, handle that
// here
//
void serializeUse(IRSerializer const& serializer, IRInst* user, IRUse& use)
{
    SLANG_ASSERT(user);
    IRInst* used = isWriting(serializer) ? use.get() : nullptr;
    serialize(serializer, used);
    if (isReading(serializer))
    {
        use.init(user, used);
    }
}

template<typename T>
void serializeObject(IRSerializer const& serializer, T*& inst, IRInst*)
{
    // Each IR instruction has:
    //
    // * An opcode
    // * Zero or more operands
    // * Zero or more children
    //
    // Most instructions are entirely defined by those properties.
    //
    // The instructions that represent simple constants (integers, strings, etc.) are
    // unique in that they have "payload" data that holds their value, instead of having
    // any operands.
    //
    // Note that as a result of the serialization strategy used by fossil, it
    // is not possible for the deserialization logic to interact with any
    // systems for deduplication or simplification of instructions.

    SLANG_SCOPED_SERIALIZER_VARIANT(serializer);

    //
    // Since we're calling deferSerializeObjectContents at the end of this
    // function we need only serialize/deserialize enough to allocate the
    // instruction itself,
    //
    // For most instructions this is simply the operand count, however for a
    // couple of exceptions (IRModuleInst and anything under IRConstant) we
    // may need to allocate more space, so first find out what sort of
    // instruction it is.
    //
    IROp op = isWriting(serializer) ? inst->m_op : kIROp_Invalid;
    uint32_t operandCount = isWriting(serializer) ? inst->operandCount : ~0;
    serialize(serializer, op);
    serialize(serializer, operandCount);

    //
    // If it's a string literal, the data is stored inline, so we need to know
    // the length of the string in order to allocate, handle that here, and we
    // may as well just read the whole string for convenience.
    //
    String stringLitString;
    if (op == kIROp_StringLit || op == kIROp_BlobLit)
    {
        if (isWriting(serializer))
        {
            stringLitString = cast<IRConstant>(inst)->getStringSlice();
        }
        serialize(serializer, stringLitString);
    }

    //
    // Now we have read/written everything we need in order to allocate the inst, do so
    // This will involve calculating the allocation size for constants also
    //
    if (isReading(serializer))
    {
        const auto readContext = static_cast<IRSerialReadContext*>(serializer.getContext());

        // We need to handle the special case instructions which aren't just defined by operands and
        // children, IRModuleInst and IRConstants
        size_t minSizeInBytes = 0;
        switch (op)
        {
        case kIROp_ModuleInst:
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
        case kIROp_StringLit:
        case kIROp_BlobLit:
            minSizeInBytes = offsetof(IRConstant, value) +
                             offsetof(IRConstant::StringValue, chars) + stringLitString.getLength();
            break;
        }
        inst = cast<T>(readContext->_module->_allocateInst(op, operandCount, minSizeInBytes));
        if (op == kIROp_StringLit || op == kIROp_BlobLit)
        {
            const auto c = cast<IRConstant>(inst);
            char* dstChars = c->value.stringVal.chars;
            c->value.stringVal.numChars = uint32_t(stringLitString.getLength());
            memcpy(dstChars, stringLitString.getBuffer(), stringLitString.getLength());
        }
    }

    // We've allocated the object, we can leave the rest for later
    deferSerializeObjectContents(serializer, inst);
}

template<typename T>
void serializeObjectContents(IRSerializer const& serializer, T*& value, IRInst*)
{
    //
    // This is all that's necessary for normal instructions
    // We serialize the source location, type, operands and children
    //
    serialize(serializer, value->sourceLoc);
    serializeUse(serializer, value, value->typeUse);
    for (Index i = 0; i < value->operandCount; ++i)
    {
        serializeUse(serializer, value, value->getOperands()[i]);
    }
    // There's an overload for this call further up in this file
    serialize(serializer, value->m_decorationsAndChildren);

    //
    // IRConstants require a little special handling
    // IRModuleInst also has some extra information, but it's just a pointer to
    // the IRModule value, and this is handled at the top level
    //
    if (const auto constant = as<IRConstant>(value))
    {
        switch (value->m_op)
        {
        case kIROp_BoolLit:
        case kIROp_IntLit:
            {
                serialize(serializer, constant->value.intVal);
            }
            break;
        case kIROp_FloatLit:
            {
                serialize(serializer, constant->value.intVal);
            }
            break;
        case kIROp_PtrLit:
            {
                // Clang gets upset using intptr_t here, due to long and long
                // long being distinct types
                auto i = reinterpret_cast<UInt64>(constant->value.ptrVal);
                serialize(serializer, i);
                constant->value.ptrVal = reinterpret_cast<void*>(i);
            }
            break;
        case kIROp_StringLit:
        case kIROp_BlobLit:
            // Since we had to read the string anyway to get the length in
            // serializeObject for this instruction, the string contents
            // have already been filled in, nothing more to do here.
            break;
        case kIROp_VoidLit:
            break;
        default:
            SLANG_UNREACHABLE("unhandled constant");
        }
    }
}

//
// Handlers for IRModule, there is a little extra setup to do once top level
// entries are deserialized to set up m_mapMangledNameToGlobalInst, this is
// done at the end of readSerializedModuleIR
//
void serializeObject(IRSerializer const& serializer, IRModule*& value, IRModule*)
{
    serializer.getContext()->handleIRModule(serializer, value);
}

void IRSerialWriteContext::handleIRModule(IRSerializer const& serializer, IRModule*& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value->m_moduleInst);
    serialize(serializer, value->m_name);
    serialize(serializer, value->m_version);
}

void IRSerialReadContext::handleIRModule(IRSerializer const& serializer, IRModule*& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    value = new IRModule{_session};
    SLANG_ASSERT(!_module);
    _module = value;
    serialize(serializer, value->m_moduleInst);
    serialize(serializer, value->m_name);
    serialize(serializer, value->m_version);
    value->m_moduleInst->module = value;
}

//
// Serialize Names via the name pool on the session, this is used just for the
// IRModule name member.
//
void serializeObject(IRSerializer const& serializer, Name*& value, Name*)
{
    serializer.getContext()->handleName(serializer, value);
}

void IRSerialWriteContext::handleName(IRSerializer const& serializer, Name*& value)
{
    serialize(serializer, value->text);
}

void IRSerialReadContext::handleName(IRSerializer const& serializer, Name*& value)
{
    String text;
    serialize(serializer, text);
    value = _session->getNamePool()->getName(text);
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
        Fossil::SerialWriter writer(blobBuilder);
        IRSerialWriteContext context{sourceLocWriter};
        IRSerializer serializer(&writer, &context);
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

//
// Read a module, this currently does not do any on-demand loading
//
Result readSerializedModuleIR(
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
    {
        auto sharedDecodingContext = RefPtr(new IRSerialReadContext(session, sourceLocReader));
        Fossil::ReadContext readContext;
        Fossil::SerialReader reader(
            readContext,
            rootValPtr,
            Fossil::SerialReader::InitialStateType::Root);

        IRSerializer serializer(&reader, sharedDecodingContext);
        serialize(serializer, info);
    }
    SLANG_ASSERT(info.module);

    //
    // Now that everything is loaded, we can traverse the module and fix up the
    // parents which we didn't do before because due to deferred
    // deserialization we didn't necessarily have this information handy at the
    // time.
    //
    bool hasUnrecognizedInsts = false;
    auto go = [&](auto&& go, IRInst* parent, IRInst* inst) -> void
    {
        if (inst->getOp() == kIROp_Unrecognized)
            hasUnrecognizedInsts = true;

        inst->parent = parent;
        for (const auto child : inst->getDecorationsAndChildren())
            go(go, inst, child);
    };
    go(go, nullptr, info.module->getModuleInst());

    if (hasUnrecognizedInsts)
        return SLANG_FAIL;

    //
    // Module is finally valid (or at least as much as it was going it) and
    // ready to be used
    //
    info.module->buildMangledNameToGlobalInstMap();
    outIRModule = info.module;
    return SLANG_OK;
}

} // namespace Slang
