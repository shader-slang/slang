#include "replay-context.h"

namespace SlangRecord {

// =============================================================================
// TypeId helpers
// =============================================================================

const char* getTypeIdName(TypeId id)
{
    switch (id)
    {
    case TypeId::Int8: return "Int8";
    case TypeId::Int16: return "Int16";
    case TypeId::Int32: return "Int32";
    case TypeId::Int64: return "Int64";
    case TypeId::UInt8: return "UInt8";
    case TypeId::UInt16: return "UInt16";
    case TypeId::UInt32: return "UInt32";
    case TypeId::UInt64: return "UInt64";
    case TypeId::Float32: return "Float32";
    case TypeId::Float64: return "Float64";
    case TypeId::Bool: return "Bool";
    case TypeId::String: return "String";
    case TypeId::Blob: return "Blob";
    case TypeId::Array: return "Array";
    case TypeId::ObjectHandle: return "ObjectHandle";
    case TypeId::Null: return "Null";
    default: return "Unknown";
    }
}

TypeMismatchException::TypeMismatchException(TypeId expected, TypeId actual)
    : std::runtime_error(std::string("Type mismatch: expected ") + 
                         getTypeIdName(expected) + ", got " + getTypeIdName(actual))
    , m_expected(expected), m_actual(actual)
{
}

// =============================================================================
// ReplayContext construction and low-level helpers
// =============================================================================

ReplayContext::ReplayContext(ReplayStream* stream, MemoryArena* arena)
    : m_stream(stream), m_arena(arena)
{
}

void ReplayContext::serializeRaw(void* data, size_t size)
{
    if (isWriting()) m_stream->write(data, size);
    else m_stream->read(data, size);
}

void ReplayContext::serializeTypeId(TypeId id)
{
    if (isWriting()) writeTypeId(id);
    else expectTypeId(id);
}

void ReplayContext::writeTypeId(TypeId id)
{
    uint8_t v = static_cast<uint8_t>(id);
    m_stream->write(&v, sizeof(v));
}

TypeId ReplayContext::readTypeId()
{
    uint8_t v;
    m_stream->read(&v, sizeof(v));
    return static_cast<TypeId>(v);
}

void ReplayContext::expectTypeId(TypeId expected)
{
    TypeId actual = readTypeId();
    if (actual != expected)
        throw TypeMismatchException(expected, actual);
}

// =============================================================================
// Basic types - integer, floating-point, boolean
// =============================================================================

void ReplayContext::serialize(int8_t& value)   { serializeTypeId(TypeId::Int8);   serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(int16_t& value)  { serializeTypeId(TypeId::Int16);  serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(int32_t& value)  { serializeTypeId(TypeId::Int32);  serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(int64_t& value)  { serializeTypeId(TypeId::Int64);  serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(uint8_t& value)  { serializeTypeId(TypeId::UInt8);  serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(uint16_t& value) { serializeTypeId(TypeId::UInt16); serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(uint32_t& value) { serializeTypeId(TypeId::UInt32); serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(uint64_t& value) { serializeTypeId(TypeId::UInt64); serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(float& value)    { serializeTypeId(TypeId::Float32); serializeRaw(&value, sizeof(value)); }
void ReplayContext::serialize(double& value)   { serializeTypeId(TypeId::Float64); serializeRaw(&value, sizeof(value)); }

void ReplayContext::serialize(bool& value)
{
    serializeTypeId(TypeId::Bool);
    if (isWriting()) { uint8_t v = value ? 1 : 0; serializeRaw(&v, sizeof(v)); }
    else { uint8_t v; serializeRaw(&v, sizeof(v)); value = (v != 0); }
}

// =============================================================================
// Strings
// =============================================================================

void ReplayContext::serialize(const char*& str)
{
    if (isWriting())
    {
        if (str == nullptr) { writeTypeId(TypeId::Null); }
        else
        {
            writeTypeId(TypeId::String);
            uint32_t length = static_cast<uint32_t>(strlen(str));
            m_stream->write(&length, sizeof(length));
            if (length > 0) m_stream->write(str, length);
        }
    }
    else
    {
        TypeId typeId = readTypeId();
        if (typeId == TypeId::Null) { str = nullptr; }
        else if (typeId == TypeId::String)
        {
            if (!m_arena) throw std::runtime_error("MemoryArena required for reading strings");
            uint32_t length;
            m_stream->read(&length, sizeof(length));
            char* buf = m_arena->allocateArray<char>(length + 1);
            if (length > 0) m_stream->read(buf, length);
            buf[length] = '\0';
            str = buf;
        }
        else { throw TypeMismatchException(TypeId::String, typeId); }
    }
}

// =============================================================================
// Blob and Handle
// =============================================================================

void ReplayContext::serializeBlob(const void*& data, size_t& size)
{
    if (isWriting())
    {
        writeTypeId(TypeId::Blob);
        uint64_t blobSize = static_cast<uint64_t>(size);
        m_stream->write(&blobSize, sizeof(blobSize));
        if (size > 0 && data != nullptr) m_stream->write(data, size);
    }
    else
    {
        expectTypeId(TypeId::Blob);
        if (!m_arena) throw std::runtime_error("MemoryArena required for reading blobs");
        uint64_t blobSize;
        m_stream->read(&blobSize, sizeof(blobSize));
        size = static_cast<size_t>(blobSize);
        if (size > 0)
        {
            void* buf = m_arena->allocate(size);
            m_stream->read(buf, size);
            data = buf;
        }
        else { data = nullptr; }
    }
}

void ReplayContext::serializeHandle(uint64_t& handleId)
{
    serializeTypeId(TypeId::ObjectHandle);
    serializeRaw(&handleId, sizeof(handleId));
}

// =============================================================================
// Slang enum types - all use serializeEnum
// =============================================================================

void ReplayContext::serialize(SlangSeverity& value)              { serializeEnum(value); }
void ReplayContext::serialize(SlangBindableResourceType& value)  { serializeEnum(value); }
void ReplayContext::serialize(SlangCompileTarget& value)         { serializeEnum(value); }
void ReplayContext::serialize(SlangContainerFormat& value)       { serializeEnum(value); }
void ReplayContext::serialize(SlangPassThrough& value)           { serializeEnum(value); }
void ReplayContext::serialize(SlangArchiveType& value)           { serializeEnum(value); }
void ReplayContext::serialize(SlangFloatingPointMode& value)     { serializeEnum(value); }
void ReplayContext::serialize(SlangFpDenormalMode& value)        { serializeEnum(value); }
void ReplayContext::serialize(SlangLineDirectiveMode& value)     { serializeEnum(value); }
void ReplayContext::serialize(SlangSourceLanguage& value)        { serializeEnum(value); }
void ReplayContext::serialize(SlangProfileID& value)             { serializeEnum(value); }
void ReplayContext::serialize(SlangCapabilityID& value)          { serializeEnum(value); }
void ReplayContext::serialize(SlangMatrixLayoutMode& value)      { serializeEnum(value); }
void ReplayContext::serialize(SlangStage& value)                 { serializeEnum(value); }
void ReplayContext::serialize(SlangDebugInfoLevel& value)        { serializeEnum(value); }
void ReplayContext::serialize(SlangDebugInfoFormat& value)       { serializeEnum(value); }
void ReplayContext::serialize(SlangOptimizationLevel& value)     { serializeEnum(value); }
void ReplayContext::serialize(SlangEmitSpirvMethod& value)       { serializeEnum(value); }
void ReplayContext::serialize(slang::CompilerOptionName& value)  { serializeEnum(value); }
void ReplayContext::serialize(slang::CompilerOptionValueKind& value) { serializeEnum(value); }
void ReplayContext::serialize(slang::ContainerType& value)       { serializeEnum(value); }
void ReplayContext::serialize(slang::SpecializationArg::Kind& value) { serializeEnum(value); }
void ReplayContext::serialize(SlangLanguageVersion& value)       { serializeEnum(value); }
void ReplayContext::serialize(slang::BuiltinModuleName& value)   { serializeEnum(value); }

// =============================================================================
// POD and complex structs
// =============================================================================

void ReplayContext::serialize(SlangUUID& value)
{
    serialize(value.data1);
    serialize(value.data2);
    serialize(value.data3);
    for (int i = 0; i < 8; ++i) serialize(value.data4[i]);
}

void ReplayContext::serialize(slang::CompilerOptionValue& value)
{
    serialize(value.kind);
    serialize(value.intValue0);
    serialize(value.intValue1);
    serialize(value.stringValue0);
    serialize(value.stringValue1);
}

void ReplayContext::serialize(slang::CompilerOptionEntry& value)
{
    serialize(value.name);
    serialize(value.value);
}

void ReplayContext::serialize(slang::PreprocessorMacroDesc& value)
{
    serialize(value.name);
    serialize(value.value);
}

void ReplayContext::serialize(slang::TargetDesc& value)
{
    uint64_t structureSize = value.structureSize;
    serialize(structureSize);
    if (isReading()) value.structureSize = static_cast<size_t>(structureSize);

    serialize(value.format);
    serialize(value.profile);
    serialize(value.flags);
    serialize(value.floatingPointMode);
    serialize(value.lineDirectiveMode);
    serialize(value.forceGLSLScalarBufferLayout);
    serializeArray(value.compilerOptionEntries, value.compilerOptionEntryCount);
}

void ReplayContext::serialize(slang::SessionDesc& value)
{
    uint64_t structureSize = value.structureSize;
    serialize(structureSize);
    if (isReading()) value.structureSize = static_cast<size_t>(structureSize);

    serializeArray(value.targets, value.targetCount);
    serialize(value.flags);
    serialize(value.defaultMatrixLayoutMode);
    serializeArray(value.searchPaths, value.searchPathCount);
    serializeArray(value.preprocessorMacros, value.preprocessorMacroCount);

    // fileSystem is handled specially - serialize as null handle for now
    uint64_t fileSystemHandle = 0;
    serializeHandle(fileSystemHandle);
    if (isReading()) value.fileSystem = nullptr;

    serialize(value.enableEffectAnnotations);
    serialize(value.allowGLSLSyntax);

    const slang::CompilerOptionEntry* entries = value.compilerOptionEntries;
    serializeArray(entries, value.compilerOptionEntryCount);
    if (isReading()) value.compilerOptionEntries = const_cast<slang::CompilerOptionEntry*>(entries);

    serialize(value.skipSPIRVValidation);
}

void ReplayContext::serialize(slang::SpecializationArg& value)
{
    serialize(value.kind);
    switch (value.kind)
    {
    case slang::SpecializationArg::Kind::Unknown: break;
    case slang::SpecializationArg::Kind::Type:
    {
        uint64_t typeHandle = 0;
        serializeHandle(typeHandle);
        if (isReading()) value.type = nullptr;
        break;
    }
    case slang::SpecializationArg::Kind::Expr:
        serialize(value.expr);
        break;
    }
}

void ReplayContext::serialize(SlangGlobalSessionDesc& value)
{
    serialize(value.structureSize);
    serialize(value.apiVersion);
    serialize(value.minLanguageVersion);
    serialize(value.enableGLSL);
    for (int i = 0; i < 16; ++i) serialize(value.reserved[i]);
}

} // namespace SlangRecord
