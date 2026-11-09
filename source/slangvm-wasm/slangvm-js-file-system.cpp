#include "slangvm-js-file-system.h"

#include "core/slang-io.h"
#include "slangvm-browser-call.h"

namespace Slang::HGVM
{
using emscripten::val;

SlangResult JavaScriptFileSystem::ensureDirectory(const String& path)
{
    if (!path.getLength() || path == ".")
        return SLANG_OK;
    SlangPathType type;
    if (SLANG_SUCCEEDED(MemoryFileSystem::getPathType(path.getBuffer(), &type)))
        return type == SLANG_PATH_TYPE_DIRECTORY ? SLANG_OK : SLANG_FAIL;
    SLANG_RETURN_ON_FAIL(ensureDirectory(Path::getParentDirectory(path)));
    return createDirectory(path.getBuffer());
}

SlangResult JavaScriptFileSystem::loadFile(const char* path, ISlangBlob** outBlob)
{
    *outBlob = nullptr;
    if (SLANG_SUCCEEDED(MemoryFileSystem::loadFile(path, outBlob)))
        return SLANG_OK;

    StringBuilder canonical;
    SLANG_RETURN_ON_FAIL(_getCanonical(path, canonical));
    String name = canonical.produceString();

    val arguments = val::array();
    arguments.call<void>("push", std::string(name.getBuffer()));
    val bytes = callBrowserHost(m_fileSystem, "loadFile", arguments);
    if (getBrowserError(bytes, m_lastError))
        return SLANG_FAIL;
    if (bytes.isNull() || bytes.isUndefined())
    {
        return SLANG_E_NOT_FOUND;
    }

    auto contents = emscripten::convertJSArrayToNumberVector<uint8_t>(bytes);
    SLANG_RETURN_ON_FAIL(ensureDirectory(Path::getParentDirectory(name)));
    SLANG_RETURN_ON_FAIL(saveFile(name.getBuffer(), contents.data(), contents.size()));
    return MemoryFileSystem::loadFile(name.getBuffer(), outBlob);
}

SlangResult JavaScriptFileSystem::getPathType(const char* path, SlangPathType* outType)
{
    if (SLANG_SUCCEEDED(MemoryFileSystem::getPathType(path, outType)))
        return SLANG_OK;
    ComPtr<ISlangBlob> contents;
    SLANG_RETURN_ON_FAIL(loadFile(path, contents.writeRef()));
    *outType = SLANG_PATH_TYPE_FILE;
    return SLANG_OK;
}

SlangResult JavaScriptFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outIdentity)
{
    SlangPathType type;
    SLANG_RETURN_ON_FAIL(getPathType(path, &type));
    return MemoryFileSystem::getFileUniqueIdentity(path, outIdentity);
}
} // namespace Slang::HGVM
