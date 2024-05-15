#include "slang-filesystem.h"
#include "capture_utility.h"

namespace SlangCapture
{
    FileSystemCapture::FileSystemCapture(ISlangFileSystemExt* fileSystem)
        : m_actualFileSystem(fileSystem)
    {
        assert(m_actualFileSystem);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, m_actualFileSystem.get());
    }

    FileSystemCapture::~FileSystemCapture()
    {
        m_actualFileSystem->release();
    }

    void* FileSystemCapture::castAs(const Slang::Guid& guid)
    {
        return getInterface(guid);
    }

    ISlangUnknown* FileSystemCapture::getInterface(const Slang::Guid& guid)
    {
        if(guid == ISlangUnknown::getTypeGuid() || guid == ISlangFileSystem::getTypeGuid())
            return static_cast<ISlangFileSystem*>(this);
        return nullptr;
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::loadFile(
                char const*     path,
                ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->loadFile(path, outBlob);
        return res;
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::getFileUniqueIdentity(
        const char* path,
        ISlangBlob** outUniqueIdentity)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s :\"%s\"\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->getFileUniqueIdentity(path, outUniqueIdentity);
        return res;
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->calcCombinedPath(fromPathType, fromPath, path, pathOut);
        return res;
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::getPathType(
        const char* path,
        SlangPathType* pathTypeOut)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->getPathType(path, pathTypeOut);
        return res;
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::getPath(
        PathKind kind,
        const char* path,
        ISlangBlob** outPath)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->getPath(kind, path, outPath);
        return res;
    }

    SLANG_NO_THROW void FileSystemCapture::clearCache()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__);
        m_actualFileSystem->clearCache();
    }

    SLANG_NO_THROW SlangResult FileSystemCapture::enumeratePathContents(
        const char* path,
        FileSystemContentsCallBack callback,
        void* userData)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->enumeratePathContents(path, callback, userData);
        return res;
    }

    SLANG_NO_THROW OSPathKind FileSystemCapture::getOSPathKind()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__);
        OSPathKind pathKind = m_actualFileSystem->getOSPathKind();
        return pathKind;
    }
}
