#ifndef SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
#define SLANG_PROXY_MUTABLE_FILE_SYSTEM_H

#include "proxy-base.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class MutableFileSystemProxy : public ISlangMutableFileSystem, public ProxyBase
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

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    SLANG_PROXY_GET_INTERFACE(ISlangMutableFileSystem)

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::castAs");
    }

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadFile(char const* path, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::loadFile");
    }

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(outUniqueIdentity);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::getFileUniqueIdentity");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) override
    {
        SLANG_UNUSED(fromPathType);
        SLANG_UNUSED(fromPath);
        SLANG_UNUSED(path);
        SLANG_UNUSED(pathOut);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::calcCombinedPath");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPathType(const char* path, SlangPathType* pathTypeOut) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(pathTypeOut);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::getPathType");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPath(PathKind kind, const char* path, ISlangBlob** outPath) override
    {
        SLANG_UNUSED(kind);
        SLANG_UNUSED(path);
        SLANG_UNUSED(outPath);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::getPath");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() override
    {
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::clearCache");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(
        const char* path,
        FileSystemContentsCallBack callback,
        void* userData) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(callback);
        SLANG_UNUSED(userData);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::enumeratePathContents");
    }

    virtual SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() override
    {
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::getOSPathKind");
    }

    // ISlangMutableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFile(const char* path, const void* data, size_t size) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(data);
        SLANG_UNUSED(size);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::saveFile");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFileBlob(const char* path, ISlangBlob* dataBlob) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(dataBlob);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::saveFileBlob");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) override
    {
        SLANG_UNUSED(path);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::remove");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) override
    {
        SLANG_UNUSED(path);
        SLANG_UNIMPLEMENTED_X("MutableFileSystemProxy::createDirectory");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
