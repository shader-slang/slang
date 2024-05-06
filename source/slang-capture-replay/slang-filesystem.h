#ifndef SLANG_FILE_SYSTEM_H
#define SLANG_FILE_SYSTEM_H

#include "../core/slang-com-object.h"
#include "../../slang-com-helper.h"

namespace SlangCapture
{

    using namespace Slang;

    // slang always requires ISlangFileSystemExt interface, even if user only provides ISlangFileSystem,
    // slang will still wrap it with ISlangFileSystemExt. So we have to capture ISlangFileSystemExt, even
    // though we only need to record loadFile() function.
    class FileSystemCapture : public RefObject, public ISlangFileSystemExt
    {
    public:
        explicit FileSystemCapture(ISlangFileSystemExt* fileSystem);
        ~FileSystemCapture();

        // ISlangUnknown
        SLANG_REF_OBJECT_IUNKNOWN_ALL

        ISlangUnknown* getInterface(const Slang::Guid& guid);

        // ISlangCastable
        virtual SLANG_NO_THROW void* castAs(const Slang::Guid& guid) override;

        // ISlangFileSystem
        virtual SLANG_NO_THROW SlangResult loadFile(
                char const*     path,
                ISlangBlob** outBlob) override;

        // ISlangFileSystemExt
        virtual SLANG_NO_THROW SlangResult getFileUniqueIdentity(
            const char* path,
            ISlangBlob** outUniqueIdentity) override;

        virtual SLANG_NO_THROW SlangResult calcCombinedPath(
            SlangPathType fromPathType,
            const char* fromPath,
            const char* path,
            ISlangBlob** pathOut) override;

        virtual SLANG_NO_THROW SlangResult getPathType(
            const char* path,
            SlangPathType* pathTypeOut) override;

        virtual SLANG_NO_THROW SlangResult getPath(
            PathKind kind,
            const char* path,
            ISlangBlob** outPath) override;

        virtual SLANG_NO_THROW void clearCache() override;

        virtual SLANG_NO_THROW SlangResult enumeratePathContents(
            const char* path,
            FileSystemContentsCallBack callback,
            void* userData) override;

        virtual SLANG_NO_THROW OSPathKind getOSPathKind() override;
    private:
        // We should release the actual file system, because if 1) the file
        // system is provided by user, we will increment the ref-count;
        // 2) if the file system is created by slang, we will transfer the ownership
        // from Linkage to FileSystemCapture, in both cases, we should release the
        // 'm_actualFileSystem'.
        ISlangFileSystemExt* m_actualFileSystem = nullptr;
};

}
#endif

