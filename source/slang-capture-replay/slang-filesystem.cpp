#include "slang-filesystem.h"
#include "capture-utility.h"
#include "output-stream.h"

namespace SlangCapture
{
    // We don't actually need to capture the methods of ISlangFileSystemExt, we just want to capture the file content
    // and save them into disk.
    FileSystemCapture::FileSystemCapture(ISlangFileSystemExt* fileSystem, CaptureManager* captureManager)
        : m_actualFileSystem(fileSystem),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualFileSystem);
        SLANG_CAPTURE_ASSERT(m_captureManager);
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

    // TODO: There could be a potential issue that could not be able to dump the generated file content correctly.
    // Details: https://github.com/shader-slang/slang/issues/4423.
    SLANG_NO_THROW SlangResult FileSystemCapture::loadFile(
                char const*     path,
                ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s, :%s\n", m_actualFileSystem.get(), __PRETTY_FUNCTION__, path);
        SlangResult res = m_actualFileSystem->loadFile(path, outBlob);

        // Since the loadFile method could be implemented by client, we can't guarantee the result is always as expected,
        // we will check every thing to make sure we won't crash at writing file.
        //
        // We can only dump the file content after this 'loadFile' call, no matter this call crashes or file is not
        // found, we can't save the file anyway, so we don't need to pay special care to the crash recovery. We will
        // know something wrong with the loadFile call if we can't find the file in the capture directory.
        if ((res == SLANG_OK) && (*outBlob != nullptr) && ((*outBlob)->getBufferSize() != 0))
        {
            std::filesystem::path filePath = m_captureManager->getCaptureFileDirectory();
            filePath = filePath / path;

            FileOutputStream fileStream(filePath.string().c_str());

            fileStream.write((*outBlob)->getBufferPointer(), (*outBlob)->getBufferSize());
            fileStream.flush();
        }
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
