#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "slang-archive-file-system.h"

namespace Slang
{

struct ZipFileSystem
{
        /// Create an empty zip
    static SlangResult create(ComPtr<IArchiveFileSystem>& out);
        /// True if this appears to be a zip archive
    static bool isArchive(const void* data, size_t size);
};

}

#endif
