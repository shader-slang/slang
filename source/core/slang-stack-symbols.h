// slang-stack-symbols.h
#ifndef SLANG_CORE_STACK_SYMBOLS_H
#define SLANG_CORE_STACK_SYMBOLS_H

#include "slang-common.h"
#include "slang-string.h"

namespace Slang
{

/// Find the name of the function that contains a code address, for the stack of a crash
/// report.
///
/// A release build of Slang cannot name most of its own functions: its symbols are hidden and
/// its debug information is in separate files. A report therefore prints each frame as
/// `module+0xoffset`, which a maintainer resolves offline. A build configured with
/// `SLANG_CRASH_REPORT_SYMBOLS=ON` also prints the function name after the offset, read from
/// what the binary itself still holds:
///
/// - On Linux, the ELF symbol table (`.symtab`), which `strip --strip-debug` keeps, or else
///   the dynamic symbol table.
/// - On macOS, `dladdr`, which reads the Mach-O symbol table that `strip -S` keeps.
/// - On Windows, DbgHelp, which needs the program's PDB, or a stripped one, beside it.
///
/// None of these gives source lines; the offline symbolizer does. The lookups read files and
/// allocate, so a report uses them only for an internal error, never from a crash handler.
///
/// A resolver keeps what it read about the last module it looked in, so resolving all the
/// frames of one stack reads each module's symbol table once.
class StackSymbolResolver
{
public:
    StackSymbolResolver();
    ~StackSymbolResolver();

    StackSymbolResolver(const StackSymbolResolver&) = delete;
    StackSymbolResolver& operator=(const StackSymbolResolver&) = delete;

    /// Return true if this build looks up names at all.
    static bool isEnabled();

    /// Write the demangled name of the function that contains `address` to `outName`, and
    /// return true. Return false if the name is not known or names are not enabled.
    bool findFunctionName(const void* address, String& outName);

private:
    struct Impl;
    Impl* m_impl = nullptr;
};

} // namespace Slang

#endif
