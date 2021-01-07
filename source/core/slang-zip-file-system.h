#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "slang-compressed-file-system.h"

namespace Slang
{

struct ZipFileSystem
{
        /// Create an empty zip
    static SlangResult create(RefPtr<CompressedFileSystem>& out);
};

}

#endif
