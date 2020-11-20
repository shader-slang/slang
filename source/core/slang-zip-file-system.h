#ifndef SLANG_ZIP_FILE_SYSTEM_H
#define SLANG_ZIP_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

class ZipFileSystem : public RefObject, public ISlangMutableFileSystem
{
public:

    virtual ArrayView<uint8_t> getArchive() = 0;

    static SlangResult create(const void* data, size_t size, ComPtr<ZipFileSystem>& out);
};

struct ZipCompressionUtil
{
    static void unitTest();

    
};

}

#endif
