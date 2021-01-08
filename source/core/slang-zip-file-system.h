#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "slang-archive-file-system.h"

namespace Slang
{

struct ZipFileSystem
{
        /// Create an empty zip
    static SlangResult create(RefPtr<ArchiveFileSystem>& out);
};

}

#endif
