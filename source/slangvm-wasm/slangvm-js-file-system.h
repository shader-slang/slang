#pragma once

#include "core/slang-memory-file-system.h"

#include <emscripten/val.h>

namespace Slang::HGVM
{
/// Caches loaded bytes from a JavaScript filesystem for one VM. Only asynchronous reads suspend
/// the WASM stack. Missing-file policy belongs to the caller's filesystem, not this adapter.
class JavaScriptFileSystem : public MemoryFileSystem
{
public:
    explicit JavaScriptFileSystem(emscripten::val fileSystem)
        : m_fileSystem(std::move(fileSystem))
    {
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    loadFile(const char* path, ISlangBlob** outBlob) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getPathType(const char* path, SlangPathType* outType) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getFileUniqueIdentity(const char* path, ISlangBlob** outIdentity) override;

    const String& getLastError() const { return m_lastError; }
    void clearLastError() { m_lastError = ""; }

private:
    SlangResult ensureDirectory(const String& path);

    emscripten::val m_fileSystem;
    String m_lastError;
};
} // namespace Slang::HGVM
