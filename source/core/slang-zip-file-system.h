#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

struct ZipCompressionUtil
{
    static void unitTest();

    static SlangResult create(const void* data, size_t size, ComPtr<ISlangMutableFileSystem>& out);
};

}

#endif
