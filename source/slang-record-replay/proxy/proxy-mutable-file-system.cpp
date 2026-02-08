#include "proxy-mutable-file-system.h"

#include "../../core/slang-blob.h"

namespace SlangRecord
{

using namespace Slang;

SlangResult MutableFileSystemProxy::loadFile(char const* path, ISlangBlob** outBlob)
{
    RECORD_CALL();
    RECORD_INPUT(path);
    PREPARE_POINTER_OUTPUT(outBlob);
    SlangResult result = SLANG_OK;
    if (ReplayContext::get().isWriting())
    {
        result = m_fileSystem->loadFile(path, outBlob);
    }
    RECORD_BLOB_OUTPUT(outBlob);
    RECORD_INFO(result);
    return result;
}

} // namespace SlangRecord
