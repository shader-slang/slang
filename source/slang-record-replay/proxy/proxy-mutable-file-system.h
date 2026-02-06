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
    // Lifetime of FS proxys is just held by the session
    // PROXY_REFCOUNT_IMPL(MutableFileSystemProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == MutableFileSystemProxy::getTypeGuid() ||
            uuid == ISlangMutableFileSystem::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangMutableFileSystem*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangFileSystemExt::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangFileSystemExt*>(static_cast<ISlangMutableFileSystem*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangFileSystem::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangFileSystem*>(static_cast<ISlangMutableFileSystem*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangCastable::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangCastable*>(static_cast<ISlangMutableFileSystem*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<ISlangMutableFileSystem*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        // Forward to underlying
        return m_fileSystem->castAs(guid);
    }

    // ISlangFileSystem
    // loadFile is special - it captures file content during recording
    // and serves from captured files during playback
    SLANG_API virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadFile(char const* path, ISlangBlob** outBlob) override;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        PREPARE_POINTER_OUTPUT(outUniqueIdentity);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_fileSystemExt->getFileUniqueIdentity(path, outUniqueIdentity);
        }

        RECORD_INFO(result);
        RECORD_BLOB_OUTPUT(outUniqueIdentity);
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
        RECORD_INPUT(fromPathType);
        RECORD_INPUT(fromPath);
        RECORD_INPUT(path);

        PREPARE_POINTER_OUTPUT(pathOut);
        
        SlangResult result;
        if(ReplayContext::get().isWriting()) 
        {        
            result = m_fileSystemExt->calcCombinedPath(fromPathType, fromPath, path, pathOut);
        }

        RECORD_INFO(result);
        RECORD_BLOB_OUTPUT(pathOut);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getPathType(const char* path, SlangPathType* pathTypeOut) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        PREPARE_POINTER_OUTPUT(pathTypeOut);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_fileSystemExt->getPathType(path, pathTypeOut);
        }

        RECORD_INFO(*pathTypeOut);
        RECORD_INFO(result);

        return result;
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

        PREPARE_POINTER_OUTPUT(outPath);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_fileSystemExt->getPath(kind, path, outPath);
        }

        RECORD_INFO(result);
        RECORD_BLOB_OUTPUT(outPath);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() override
    {
        if (!m_fileSystemExt)
            return;

        RECORD_CALL();
        if (ReplayContext::get().isWriting())
        {
            m_fileSystemExt->clearCache();
        }
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(
        const char* path,
        FileSystemContentsCallBack callback,
        void* userData) override
    {
        if (!m_fileSystemExt)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        SlangResult result = SLANG_OK;
        
        if (ReplayContext::get().isWriting())
        {
            // Wrapper context to capture and record each entry during enumeration
            struct WrapperContext {
                FileSystemContentsCallBack originalCallback;
                void* originalUserData;                
                static void callback(SlangPathType pathType, const char* name, void* userData) {
                    auto& _ctx = ReplayContext::get();
                    RECORD_INFO(pathType);
                    RECORD_INFO(name);
                    auto* wrapper = static_cast<WrapperContext*>(userData);
                    if (wrapper->originalCallback)
                        wrapper->originalCallback(pathType, name, wrapper->originalUserData);
                }
            };
            
            // Call with wrapper, which records each entry before forwarding to the original callback
            WrapperContext wrapper = { callback, userData };
            result = m_fileSystemExt->enumeratePathContents(path, WrapperContext::callback, &wrapper);
            
            // Write sentinel to mark end of entries
            SlangPathType sentinalPathType = static_cast<SlangPathType>(-1);
            RECORD_INFO(sentinalPathType);
        }
        else 
        {
            // Read entries from stream and replay callbacks
            while (true)
            {
                SlangPathType pathType;
                RECORD_INFO(pathType);
                if(pathType == static_cast<SlangPathType>(-1))
                    break; // Sentinel reached, enumeration complete
                const char* name = nullptr;
                RECORD_INFO(name);
                if (callback)
                    callback(pathType, name, userData);
            }            
        }
        
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() override
    {
        if (!m_fileSystemExt)
            return OSPathKind::None;

        RECORD_CALL();
        OSPathKind result;
        if (ReplayContext::get().isWriting())
        {
            result = m_fileSystemExt->getOSPathKind();
        }
        RECORD_INFO(result);
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

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_mutableFileSystem->saveFile(path, data, size);
        }
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveFileBlob(const char* path, ISlangBlob* dataBlob) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_mutableFileSystem->saveFileBlob(path, dataBlob);
        }
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_mutableFileSystem->remove(path);
        }
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) override
    {
        if (!m_mutableFileSystem)
            return SLANG_E_NOT_IMPLEMENTED;

        RECORD_CALL();
        RECORD_INPUT(path);

        SlangResult result;
        if (ReplayContext::get().isWriting())
        {
            result = m_mutableFileSystem->createDirectory(path);
        }
        RECORD_INFO(result);
        return result;
    }

private:
    ISlangFileSystem* m_fileSystem;           // Always valid
    ISlangFileSystemExt* m_fileSystemExt;     // May be null
    ISlangMutableFileSystem* m_mutableFileSystem; // May be null
};

} // namespace SlangRecord

#endif // SLANG_PROXY_MUTABLE_FILE_SYSTEM_H
