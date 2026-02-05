#ifndef SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
#define SLANG_PROXY_MUTABLE_FILE_SYSTEM_H

#include "proxy-base.h"
#include "proxy-macros.h"
#include "../replay-context.h"

namespace SlangRecord
{

class MutableFileSystemProxy : public ProxyBase<ISlangMutableFileSystem>
{
public:
    SLANG_COM_INTERFACE(
        0x809f2b7e,
        0x5c6d,
        0x8e3f,
        {0xc0, 0xb1, 0xac, 0x7d, 0x6e, 0x5f, 0x40, 0xd1})

    explicit MutableFileSystemProxy(ISlangMutableFileSystem* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(MutableFileSystemProxy)

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        // Forward to underlying - no need to record this
        return getActual<ISlangMutableFileSystem>()->castAs(guid);
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
        RECORD_CALL();
        RECORD_INPUT(path);

        ISlangBlob* identityPtr = nullptr;
        if (!outUniqueIdentity)
            outUniqueIdentity = &identityPtr;

        auto result =
            getActual<ISlangMutableFileSystem>()->getFileUniqueIdentity(path, outUniqueIdentity);

        RECORD_COM_OUTPUT(outUniqueIdentity);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) override
    {
        RECORD_CALL();
        uint32_t fromPathTypeVal = static_cast<uint32_t>(fromPathType);
        _ctx.record(RecordFlag::Input, fromPathTypeVal);
        RECORD_INPUT(fromPath);
        RECORD_INPUT(path);

        ISlangBlob* pathPtr = nullptr;
        if (!pathOut)
            pathOut = &pathPtr;

        auto result = getActual<ISlangMutableFileSystem>()->calcCombinedPath(
            fromPathType,
            fromPath,
            path,
            pathOut);

        RECORD_COM_OUTPUT(pathOut);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPathType(const char* path, SlangPathType* pathTypeOut) override
    {
        RECORD_CALL();
        RECORD_INPUT(path);

        SlangPathType pathType = SLANG_PATH_TYPE_FILE;
        if (!pathTypeOut)
            pathTypeOut = &pathType;

        auto result = getActual<ISlangMutableFileSystem>()->getPathType(path, pathTypeOut);

        uint32_t pathTypeVal = static_cast<uint32_t>(*pathTypeOut);
        _ctx.record(RecordFlag::Output, pathTypeVal);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPath(PathKind kind, const char* path, ISlangBlob** outPath) override
    {
        RECORD_CALL();
        uint8_t kindVal = static_cast<uint8_t>(kind);
        _ctx.record(RecordFlag::Input, kindVal);
        RECORD_INPUT(path);

        ISlangBlob* pathPtr = nullptr;
        if (!outPath)
            outPath = &pathPtr;

        auto result = getActual<ISlangMutableFileSystem>()->getPath(kind, path, outPath);

        RECORD_COM_OUTPUT(outPath);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() override
    {
        RECORD_CALL();
        getActual<ISlangMutableFileSystem>()->clearCache();
        RECORD_RETURN_VOID();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(
        const char* path,
        FileSystemContentsCallBack callback,
        void* userData) override
    {
        // Callbacks are complex to serialize - just forward without recording
        return getActual<ISlangMutableFileSystem>()->enumeratePathContents(
            path,
            callback,
            userData);
    }

    virtual SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() override
    {
        RECORD_CALL();
        auto result = getActual<ISlangMutableFileSystem>()->getOSPathKind();
        uint8_t resultVal = static_cast<uint8_t>(result);
        _ctx.record(RecordFlag::ReturnValue, resultVal);
        return result;
    }

    // ISlangMutableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFile(const char* path, const void* data, size_t size) override
    {
        RECORD_CALL();
        RECORD_INPUT(path);
        const void* dataPtr = data;
        size_t dataSize = size;
        _ctx.recordBlob(RecordFlag::Input, dataPtr, dataSize);

        auto result = getActual<ISlangMutableFileSystem>()->saveFile(path, data, size);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFileBlob(const char* path, ISlangBlob* dataBlob) override
    {
        RECORD_CALL();
        RECORD_INPUT(path);
        RECORD_INPUT(dataBlob);

        auto result = getActual<ISlangMutableFileSystem>()->saveFileBlob(path, dataBlob);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) override
    {
        RECORD_CALL();
        RECORD_INPUT(path);

        auto result = getActual<ISlangMutableFileSystem>()->remove(path);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) override
    {
        RECORD_CALL();
        RECORD_INPUT(path);

        auto result = getActual<ISlangMutableFileSystem>()->createDirectory(path);
        RECORD_RETURN(result);
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
