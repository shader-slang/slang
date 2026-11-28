// slang-win-stack-symbols.cpp
#include "core/slang-stack-symbols.h"

#if SLANG_CRASH_REPORT_SYMBOLS
#include <mutex>
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#endif

namespace Slang
{

#if SLANG_CRASH_REPORT_SYMBOLS

// DbgHelp keeps one state for the whole process and allows one caller at a time, so every
// call goes through this lock. A host application that uses DbgHelp itself shares that state,
// which is why names are a build option and not the default.
static std::mutex g_dbgHelpMutex;
static bool g_isDbgHelpInitialized = false;

struct StackSymbolResolver::Impl
{
};

StackSymbolResolver::StackSymbolResolver() {}

StackSymbolResolver::~StackSymbolResolver() {}

/* static */ bool StackSymbolResolver::isEnabled()
{
    return true;
}

bool StackSymbolResolver::findFunctionName(const void* address, String& outName)
{
    std::lock_guard<std::mutex> lock(g_dbgHelpMutex);
    const HANDLE process = ::GetCurrentProcess();
    if (!g_isDbgHelpInitialized)
    {
        // Undecorated names, and symbols loaded only for the modules that a report needs.
        // DbgHelp looks for each module's PDB next to it, where the release packages put one.
        ::SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        if (!::SymInitialize(process, nullptr, TRUE))
            return false;
        g_isDbgHelpInitialized = true;
    }

    alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
    memset(symbol, 0, sizeof(SYMBOL_INFO));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    if (!::SymFromAddr(process, DWORD64(uintptr_t(address)), &displacement, symbol))
        return false;
    outName = String(UnownedStringSlice(symbol->Name, symbol->NameLen));
    return true;
}

#else

// Names are not enabled in this build.

struct StackSymbolResolver::Impl
{
};

StackSymbolResolver::StackSymbolResolver() {}

StackSymbolResolver::~StackSymbolResolver() {}

/* static */ bool StackSymbolResolver::isEnabled()
{
    return false;
}

bool StackSymbolResolver::findFunctionName(const void* address, String& outName)
{
    SLANG_UNUSED(address);
    SLANG_UNUSED(outName);
    return false;
}

#endif

} // namespace Slang
