#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

class CompressedFileSystem : public RefObject, public ISlangMutableFileSystem
{
public:

    enum class CompressionType
    {
        BestSpeed,
        BestCompression,
    };

        /// Get as an archive (that can be saved to disk)
    virtual ConstArrayView<uint8_t> getArchive() = 0;
        /// Set the compression - used for any subsequent items added
    virtual void setCompressionType(CompressionType type) = 0;

        /// Create a zip with the contents of data/size (the contents of a zip file)
    static SlangResult createZip(const void* data, size_t size, RefPtr<CompressedFileSystem>& out);

        /// Create an empty zip
    static SlangResult createZip(RefPtr<CompressedFileSystem>& out);
};

}

#endif
