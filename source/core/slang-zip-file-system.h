#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "slang-compressed-file-system.h"

namespace Slang
{

struct ZipFileSystem
{
        /// Create a zip with the contents of data/size (the contents of a zip file)
    static SlangResult create(const void* data, size_t size, RefPtr<CompressedFileSystem>& out);

        /// Create an empty zip
    static SlangResult create(RefPtr<CompressedFileSystem>& out);
};

}

#endif
