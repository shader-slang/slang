#ifndef SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
#define SLANG_PROXY_MUTABLE_FILE_SYSTEM_H

#include "proxy-base.h"
#include "proxy-macros.h"
#include "../replay-context.h"

namespace SlangRecord
{

/// Proxy that wraps any file system (ISlangFileSystem, ISlangFileSystemExt, or
/// ISlangMutableFileSystem) and provides the full ISlangMutableFileSystem interface.
/// Methods not supported by the underlying file system return SLANG_E_NOT_IMPLEMENTED.
class MutableFileSystemProxy : public ProxyBase<ISlangMutableFileSystem>
{
public:
    SLANG_COM_INTERFACE(
        0x809f2b7e,
        0x5c6d,
        0x8e3f,
        {0xc0, 0xb1, 0xac, 0x7d, 0x6e, 0x5f, 0x40, 0xd1})

    /// Construct from any level of file system interface
    explicit MutableFileSystemProxy(ISlangFileSystem* actual)
        : ProxyBase(actual)
        , m_fileSystem(actual)
        , m_fileSystemExt(nullptr)
        , m_mutableFileSystem(nullptr)
    {
        // Query for extended interfaces
        m_fileSystemExt = static_cast<ISlangFileSystemExt*>(
            actual->castAs(ISlangFileSystemExt::getTypeGuid()));
        if (m_fileSystemExt)
        {
            m_mutableFileSystem = static_cast<ISlangMutableFileSystem*>(
                actual->castAs(ISlangMutableFileSystem::getTypeGuid()));
        }
    }

    explicit MutableFileSystemProxy(ISlangFileSystemExt* actual)
        : ProxyBase(actual)
        , m_fileSystem(actual)
        , m_fileSystemExt(actual)
        , m_mutableFileSystem(nullptr)
    {
        m_mutableFileSystem = static_cast<ISlangMutableFileSystem*>(
            actual->castAs(ISlangMutableFileSystem::getTypeGuid()));
    }

    explicit MutableFileSystemProxy(ISlangMutableFileSystem* actual)
        : ProxyBase(actual)
        , m_fileSystem(actual)
        , m_fileSystemExt(actual)
        , m_mutableFileSystem(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(MutableFileSystemProxy)

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        // Forward to underlying
        return m_fileSystem->castAs(guid);
    }

    // ISlangFileSystem
    // loadFile is special - it captures file content during recording
    // and serves from captured files during playback
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadFile(char const* path, ISlangBlob** outBlob) override;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        ISlangBlob* identityPtr = nullptr;
        if (!outUniqueIdentity)
            outUniqueIdentity = &identityPtr;

        auto result = m_fileSystemExt->getFileUniqueIdentity(path, outUniqueIdentity);

        RECORD_COM_OUTPUT(outUniqueIdentity);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        uint32_t fromPathTypeVal = static_cast<uint32_t>(fromPathType);
        _ctx.record(RecordFlag::Input, fromPathTypeVal);
        RECORD_INPUT(fromPath);
        RECORD_INPUT(path);

        ISlangBlob* pathPtr = nullptr;
        if (!pathOut)
            pathOut = &pathPtr;

        auto result = m_fileSystemExt->calcCombinedPath(fromPathType, fromPath, path, pathOut);

        RECORD_COM_OUTPUT(pathOut);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPathType(const char* path, SlangPathType* pathTypeOut) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        SlangPathType pathType = SLANG_PATH_TYPE_FILE;
        if (!pathTypeOut)
            pathTypeOut = &pathType;

        auto result = m_fileSystemExt->getPathType(path, pathTypeOut);

        uint32_t pathTypeVal = static_cast<uint32_t>(*pathTypeOut);
        _ctx.record(RecordFlag::Output, pathTypeVal);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPath(PathKind kind, const char* path, ISlangBlob** outPath) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        uint8_t kindVal = static_cast<uint8_t>(kind);
        _ctx.record(RecordFlag::Input, kindVal);
        RECORD_INPUT(path);

        ISlangBlob* pathPtr = nullptr;
        if (!outPath)
            outPath = &pathPtr;

        auto result = m_fileSystemExt->getPath(kind, path, outPath);

        RECORD_COM_OUTPUT(outPath);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() override
    {
        if (!m_fileSystemExt)
            return;

        RECORD_CALL();
        m_fileSystemExt->clearCache();
        RECORD_RETURN_VOID();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(
        const char* path,
        FileSystemContentsCallBack callback,
        void* userData) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        // Callbacks are complex to serialize - just forward without recording
        return m_fileSystemExt->enumeratePathContents(path, callback, userData);
    }

    virtual SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() override
    {
        if (!m_fileSystemExt)
            return OSPathKind::None;

        RECORD_CALL();
        auto result = m_fileSystemExt->getOSPathKind();
        uint8_t resultVal = static_cast<uint8_t>(result);
        _ctx.record(RecordFlag::ReturnValue, resultVal);
        return result;
    }

    // ISlangMutableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFile(const char* path, const void* data, size_t size) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);
        const void* dataPtr = data;
        size_t dataSize = size;
        _ctx.recordBlob(RecordFlag::Input, dataPtr, dataSize);

        auto result = m_mutableFileSystem->saveFile(path, data, size);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFileBlob(const char* path, ISlangBlob* dataBlob) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);
        RECORD_INPUT(dataBlob);

        auto result = m_mutableFileSystem->saveFileBlob(path, dataBlob);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        auto result = m_mutableFileSystem->remove(path);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        auto result = m_mutableFileSystem->createDirectory(path);
        RECORD_RETURN(result);
    }

private:
    ISlangFileSystem* m_fileSystem;           // Always valid
    ISlangFileSystemExt* m_fileSystemExt;     // May be null
    ISlangMutableFileSystem* m_mutableFileSystem; // May be null
};

} // namespace SlangRecord

#endif // SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
