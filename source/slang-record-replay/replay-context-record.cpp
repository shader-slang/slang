#include "replay-context.h"

#include "../core/slang-blob.h"
#include "../core/slang-crypto.h"
#include "../core/slang-io.h"
#include "../core/slang-platform.h"
#include "../slang/slang-ast-type.h"
#include "../slang/slang-compiler-api.h"
#include "../slang/slang-syntax.h"
#include "proxy/proxy-component-type.h"

#include <chrono>
#include <cinttypes>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

namespace SlangRecord
{

using Slang::File;
using Slang::Path;


void ReplayContext::recordRaw(RecordFlag flags, void* data, size_t size)
{
    switch (m_mode)
    {
    case Mode::Idle:
        // No-op in idle mode
        break;

    case Mode::Record:
        // Write data to stream
        m_stream.write(data, size);
        break;

    case Mode::Sync:
        {
            // Write data to stream
            m_stream.write(data, size);

            // Compare against reference stream using reusable buffer
            size_t offset = m_referenceStream.getPosition();
            if (size_t(m_compareBuffer.getCount()) < size)
                m_compareBuffer.setCount(Slang::Index(size));
            m_referenceStream.read(m_compareBuffer.getBuffer(), size);

            if (memcmp(data, m_compareBuffer.getBuffer(), size) != 0)
            {
                throw DataMismatchException(offset, size);
            }
        }
        break;

    case Mode::Playback:
        // For output parameters and return values, the user has provided
        // what they expect the output to be. We read the recorded value
        // and compare it against the user's expected value.
        if (hasFlag(flags, RecordFlag::Output) || hasFlag(flags, RecordFlag::ReturnValue))
        {
            // Save the user-provided expected value
            if (size_t(m_compareBuffer.getCount()) < size)
                m_compareBuffer.setCount(Slang::Index(size));
            memcpy(m_compareBuffer.getBuffer(), data, size);

            // Read the recorded value from stream
            size_t offset = m_stream.getPosition();
            m_stream.read(data, size);

            // Compare: recorded value (now in data) vs expected value (in buffer)
            if (memcmp(data, m_compareBuffer.getBuffer(), size) != 0)
            {
                throw DataMismatchException(offset, size);
            }
        }
        else
        {
            // For inputs, just read from stream
            m_stream.read(data, size);
        }
        break;
    }
}

void ReplayContext::recordTypeId(TypeId id)
{
    switch (m_mode)
    {
    case Mode::Idle:
        break;
    case Mode::Record:
        writeTypeId(id);
        break;
    case Mode::Sync:
        {
            // Write to main stream
            writeTypeId(id);
            // Verify against reference stream
            TypeId refId = readTypeIdFromReference();
            if (refId != id)
                throw TypeMismatchException(refId, id);
        }
        break;
    case Mode::Playback:
        expectTypeId(id);
        break;
    }
}

void ReplayContext::writeTypeId(TypeId id)
{
    uint8_t v = static_cast<uint8_t>(id);
    m_stream.write(&v, sizeof(v));
}

TypeId ReplayContext::readTypeId()
{
    uint8_t v;
    m_stream.read(&v, sizeof(v));
    return static_cast<TypeId>(v);
}

TypeId ReplayContext::readTypeIdFromReference()
{
    uint8_t v;
    m_referenceStream.read(&v, sizeof(v));
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

void ReplayContext::record(RecordFlag flags, int8_t& value)
{
    recordTypeId(TypeId::Int8);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, int16_t& value)
{
    recordTypeId(TypeId::Int16);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, int32_t& value)
{
    recordTypeId(TypeId::Int32);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, int64_t& value)
{
    recordTypeId(TypeId::Int64);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, uint8_t& value)
{
    recordTypeId(TypeId::UInt8);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, uint16_t& value)
{
    recordTypeId(TypeId::UInt16);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, uint32_t& value)
{
    recordTypeId(TypeId::UInt32);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, uint64_t& value)
{
    recordTypeId(TypeId::UInt64);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, float& value)
{
    recordTypeId(TypeId::Float32);
    recordRaw(flags, &value, sizeof(value));
}
void ReplayContext::record(RecordFlag flags, double& value)
{
    recordTypeId(TypeId::Float64);
    recordRaw(flags, &value, sizeof(value));
}

void ReplayContext::record(RecordFlag flags, bool& value)
{
    if (m_mode == Mode::Idle)
        return;
    recordTypeId(TypeId::Bool);
    if (isWriting())
    {
        uint8_t v = value ? 1 : 0;
        recordRaw(flags, &v, sizeof(v));
    }
    else
    {
        uint8_t v;
        recordRaw(flags, &v, sizeof(v));
        value = (v != 0);
    }
}

// =============================================================================
// Strings
// =============================================================================

void ReplayContext::record(RecordFlag flags, const char*& str)
{
    if (m_mode == Mode::Idle)
        return;
    if (isWriting())
    {
        if (str == nullptr)
        {
            recordTypeId(TypeId::Null);
        }
        else
        {
            recordTypeId(TypeId::String);
            uint32_t length = static_cast<uint32_t>(strlen(str));
            recordRaw(RecordFlag::None, &length, sizeof(length));
            if (length > 0)
                recordRaw(RecordFlag::None, const_cast<char*>(str), length);
        }
    }
    else
    {
        TypeId typeId = readTypeId();
        if (typeId == TypeId::Null)
        {
            str = nullptr;
        }
        else if (typeId == TypeId::String)
        {
            uint32_t length;
            recordRaw(RecordFlag::None, &length, sizeof(length));
            char* buf = m_arena.allocateArray<char>(length + 1);
            if (length > 0)
                recordRaw(RecordFlag::None, buf, length);
            buf[length] = '\0';
            if (hasFlag(flags, RecordFlag::Output))
            {
                if (strcmp(str, buf) != 0)
                {
                    throw DataMismatchException(m_stream.getPosition() - length, length);
                }
            }
            str = buf;
        }
        else
        {
            throw TypeMismatchException(TypeId::String, typeId);
        }
    }
}

// =============================================================================
// Blob and Handle
// =============================================================================

void ReplayContext::recordBlobByHash(RecordFlag flags, ISlangBlob*& blob)
{
    SLANG_UNUSED(flags);
    if (m_mode == Mode::Idle)
        return;

    if (isWriting())
    {
        recordTypeId(TypeId::Blob);

        if (blob == nullptr)
        {
            // Write empty hash for null blob
            const char* emptyHash = "";
            record(RecordFlag::None, emptyHash);
            return;
        }

        // Compute SHA1 hash of blob content
        Slang::SHA1::Digest digest =
            Slang::SHA1::compute(blob->getBufferPointer(), blob->getBufferSize());
        Slang::String hash = digest.toString();

        // Store blob content to disk by hash (de-duplicated)
        const char* replayPath = getCurrentReplayPath();
        if (replayPath)
        {
            Slang::String filesDir = Slang::Path::combine(Slang::String(replayPath), "files");
            Slang::Path::createDirectoryRecursive(filesDir);

            Slang::String contentPath = Slang::Path::combine(filesDir, hash);
            if (!Slang::File::exists(contentPath))
            {
                Slang::File::writeAllBytes(
                    contentPath,
                    blob->getBufferPointer(),
                    blob->getBufferSize());
            }
        }

        // Record the hash in the stream
        const char* hashCStr = hash.getBuffer();
        record(RecordFlag::None, hashCStr);
    }
    else
    {
        // Playback: read hash and load blob from disk
        expectTypeId(TypeId::Blob);

        const char* hashCStr = nullptr;
        record(RecordFlag::None, hashCStr);

        if (!hashCStr || hashCStr[0] == '\0')
        {
            blob = nullptr;
            return;
        }

        // Load content from captured file
        const char* replayPath = getCurrentReplayPath();
        if (!replayPath)
        {
            blob = nullptr;
            return;
        }

        Slang::String filesDir = Slang::Path::combine(Slang::String(replayPath), "files");
        Slang::String contentPath = Slang::Path::combine(filesDir, Slang::String(hashCStr));

        Slang::List<uint8_t> fileData;
        SlangResult readResult = Slang::File::readAllBytes(contentPath, fileData);
        if (SLANG_FAILED(readResult))
        {
            blob = nullptr;
            return;
        }

        // Create blob from loaded content
        Slang::ComPtr<ISlangBlob> loadedBlob =
            Slang::RawBlob::create(fileData.getBuffer(), fileData.getCount());
        blob = loadedBlob.detach();
    }
}

void ReplayContext::recordHandle(RecordFlag flags, uint64_t& handleId)
{
    if (m_mode == Mode::Idle)
        return;
    recordTypeId(TypeId::ObjectHandle);
    recordRaw(flags, &handleId, sizeof(handleId));
}

// =============================================================================
// Slang enum types - all use recordEnum
// =============================================================================

void ReplayContext::record(RecordFlag flags, SlangSeverity& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangParameterCategory& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangBindableResourceType& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangCompileTarget& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangContainerFormat& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangPassThrough& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangArchiveType& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangFloatingPointMode& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangFpDenormalMode& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangLineDirectiveMode& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangSourceLanguage& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangProfileID& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangCapabilityID& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangMatrixLayoutMode& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangStage& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangDebugInfoLevel& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangDebugInfoFormat& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangOptimizationLevel& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangPathType& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, OSPathKind& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangEmitSpirvMethod& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::LayoutRules& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::CompilerOptionName& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::CompilerOptionValueKind& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::ContainerType& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::SpecializationArg::Kind& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, SlangLanguageVersion& value)
{
    recordEnum(flags, value);
}
void ReplayContext::record(RecordFlag flags, slang::BuiltinModuleName& value)
{
    recordEnum(flags, value);
}

// =============================================================================
// POD and complex structs
// =============================================================================

void ReplayContext::record(RecordFlag flags, SlangUUID& value)
{
    record(flags, value.data1);
    record(flags, value.data2);
    record(flags, value.data3);
    for (int i = 0; i < 8; ++i)
        record(flags, value.data4[i]);
}

void ReplayContext::record(RecordFlag flags, slang::CompilerOptionValue& value)
{
    record(flags, value.kind);
    record(flags, value.intValue0);
    record(flags, value.intValue1);
    record(flags, value.stringValue0);
    record(flags, value.stringValue1);
}

void ReplayContext::record(RecordFlag flags, slang::CompilerOptionEntry& value)
{
    record(flags, value.name);
    record(flags, value.value);
}

void ReplayContext::record(RecordFlag flags, slang::PreprocessorMacroDesc& value)
{
    record(flags, value.name);
    record(flags, value.value);
}

void ReplayContext::record(RecordFlag flags, slang::TargetDesc& value)
{
    uint64_t structureSize = value.structureSize;
    record(flags, structureSize);
    if (isReading())
        value.structureSize = static_cast<size_t>(structureSize);

    record(flags, value.format);
    record(flags, value.profile);
    record(flags, value.flags);
    record(flags, value.floatingPointMode);
    record(flags, value.lineDirectiveMode);
    record(flags, value.forceGLSLScalarBufferLayout);
    recordArray(flags, value.compilerOptionEntries, value.compilerOptionEntryCount);
}

void ReplayContext::record(RecordFlag flags, slang::SessionDesc& value)
{
    uint64_t structureSize = value.structureSize;
    record(flags, structureSize);
    if (isReading())
        value.structureSize = static_cast<size_t>(structureSize);

    recordArray(flags, value.targets, value.targetCount);
    record(flags, value.flags);
    record(flags, value.defaultMatrixLayoutMode);
    recordArray(flags, value.searchPaths, value.searchPathCount);
    recordArray(flags, value.preprocessorMacros, value.preprocessorMacroCount);

    // fileSystem is handled specially - record as null handle for now
    uint64_t fileSystemHandle = 0;
    recordHandle(flags, fileSystemHandle);
    if (isReading())
        value.fileSystem = nullptr;

    record(flags, value.enableEffectAnnotations);
    record(flags, value.allowGLSLSyntax);

    const slang::CompilerOptionEntry* entries = value.compilerOptionEntries;
    recordArray(flags, entries, value.compilerOptionEntryCount);
    if (isReading())
        value.compilerOptionEntries = const_cast<slang::CompilerOptionEntry*>(entries);

    record(flags, value.skipSPIRVValidation);
}

void ReplayContext::record(RecordFlag flags, slang::SpecializationArg& value)
{
    record(flags, value.kind);
    switch (value.kind)
    {
    case slang::SpecializationArg::Kind::Unknown:
        break;
    case slang::SpecializationArg::Kind::Type:
        {
            record(flags, value.type);
            break;
        }
    case slang::SpecializationArg::Kind::Expr:
        record(flags, value.expr);
        break;
    }
}

void ReplayContext::record(RecordFlag flags, SlangGlobalSessionDesc& value)
{
    record(flags, value.structureSize);
    record(flags, value.apiVersion);
    record(flags, value.minLanguageVersion);
    record(flags, value.enableGLSL);
    for (int i = 0; i < 16; ++i)
        record(flags, value.reserved[i]);
}

// =============================================================================
// COM Interface record() overloads - each delegates to recordInterfaceImpl
// =============================================================================

void ReplayContext::record(RecordFlag flags, ISlangBlob*& obj)
{
    recordBlobByHash(flags, obj);
}

void ReplayContext::record(RecordFlag flags, ISlangFileSystem*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, ISlangFileSystemExt*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, ISlangMutableFileSystem*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, ISlangSharedLibrary*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::IGlobalSession*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::ISession*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::IModule*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::IComponentType*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::IEntryPoint*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::ITypeConformance*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::ICompileRequest*& obj)
{
    recordInterfaceImpl(flags, obj);
}

void ReplayContext::record(RecordFlag flags, slang::TypeReflection*& type)
{
    if (m_mode == Mode::Idle)
        return;

    if (isWriting())
    {
        recordTypeId(TypeId::TypeReflectionRef);

        // Handle null type
        if (type == nullptr)
        {
            uint64_t nullHandle = kNullHandle;
            recordHandle(flags, nullHandle);
            return;
        }

        // Get the type's full name
        Slang::ComPtr<ISlangBlob> nameBlob;
        type->getFullName(nameBlob.writeRef());
        const char* typeName = nameBlob ? (const char*)nameBlob->getBufferPointer() : nullptr;

        // Go via decl ref to get to the module that owns the type, and from there its module
        Slang::DeclRefType* declRefType = Slang::as<Slang::DeclRefType>(Slang::asInternal(type));
        Slang::Module* owningModule = Slang::getModule(declRefType->getDeclRef().getDecl());

        // Get the module handle (the module should already be registered)
        uint64_t moduleHandle = kNullHandle;
        if (owningModule)
        {
            // Module implements slang::IModule, so we can cast it
            ComPtr<slang::IComponentType> componentTypeInterface;
            if (owningModule->queryInterface(
                    slang::IComponentType::getTypeGuid(),
                    (void**)componentTypeInterface.writeRef()) == SLANG_OK)
            {
                auto proxy = getProxy(componentTypeInterface.get());
                moduleHandle = getProxyHandle(proxy);
            }
        }

        // HACK! The module wasn't found, which means this was probably a builtin type that
        // was looked up via a user loaded module's layout. The only way we can currently
        // handle this is to go over our loaded modules and find one that contains it
        if (!moduleHandle)
        {
            for (auto& kv : m_implToProxy)
            {
                // This 'safe' cast is applied to the proxy, which we know will always be some
                // valid virtual ISlangUnknown pointer, even if its not a module, so won't break DC.
                ComPtr<slang::IComponentType> componentTypeInterface;
                if (kv.first->queryInterface(
                        slang::IComponentType::getTypeGuid(),
                        (void**)componentTypeInterface.writeRef()) == SLANG_OK)
                {
                    auto layout = componentTypeInterface->getLayout(0, nullptr);
                    if (layout && layout->findTypeByName(typeName) == type)
                    {
                        auto proxy = getProxy(componentTypeInterface.get());
                        moduleHandle = getProxyHandle(proxy);
                        break;
                    }
                }
            }
        }

        if (!moduleHandle)
        {
            // If we still don't have a module handle, we can't record this type reference
            throw UnresolvedTypeException(type);
        }

        // Record the module handle and type name
        recordHandle(flags, moduleHandle);
        record(flags, typeName);
    }
    else
    {
        // Playback mode
        expectTypeId(TypeId::TypeReflectionRef);

        uint64_t moduleHandle = kNullHandle;
        recordHandle(flags, moduleHandle);

        if (moduleHandle == kNullHandle)
        {
            type = nullptr;
            return;
        }

        // Read the type name
        const char* typeName = nullptr;
        record(flags, typeName);

        // Look up the module from the handle
        ISlangUnknown* moduleUnknown = getProxy(moduleHandle);

        // Unwrap proxy if needed
        ISlangUnknown* impl = getImplementation(moduleUnknown);
        if (impl)
            moduleUnknown = impl;

        // Query for IModule interface properly (handles multiple inheritance)
        Slang::ComPtr<slang::IComponentType> moduleInterface;
        if (moduleUnknown)
        {
            moduleUnknown->queryInterface(
                slang::IComponentType::getTypeGuid(),
                (void**)moduleInterface.writeRef());
        }

        // Get the type via the module's layout
        if (moduleInterface && typeName)
        {
            auto* layout = moduleInterface->getLayout(0, nullptr);
            if (layout)
            {
                type = layout->findTypeByName(typeName);
            }
            else
            {
                type = nullptr;
                throw UnresolvedTypeException(type);
            }
        }
        else
        {
            type = nullptr;
            throw UnresolvedTypeException(type);
        }
    }
}

void ReplayContext::record(RecordFlag flags, slang::ProgramLayout*& type)
{
     if (m_mode == Mode::Idle)
        return;

    if (isWriting())
    {
    }
    else
    {
        
    }
   
}


} // namespace SlangRecord
