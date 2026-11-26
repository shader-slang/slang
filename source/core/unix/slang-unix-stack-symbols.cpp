// slang-unix-stack-symbols.cpp
#include "core/slang-list.h"
#include "core/slang-stack-symbols.h"

#if SLANG_CRASH_REPORT_SYMBOLS && !SLANG_WASM
#include <cxxabi.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if SLANG_LINUX_FAMILY
#include <elf.h>
#endif
#endif

namespace Slang
{

#if SLANG_CRASH_REPORT_SYMBOLS && !SLANG_WASM

/// Append `symbol` to `outName`, demangled if it is a mangled C++ name.
static void _appendDemangled(const char* symbol, String& outName)
{
    int status = 0;
    char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
    if (status == 0 && demangled)
    {
        outName = demangled;
        free(demangled);
        return;
    }
    free(demangled);
    outName = symbol;
}

#if SLANG_LINUX_FAMILY

/// The function symbols of one ELF module, read from its file.
struct ElfFunctionTable
{
    String path;
    /// The virtual address that the start of the loaded module has in the file.
    uint64_t fileBase = 0;
    List<Elf64_Sym> symbols;
    List<char> names;
    bool isValid = false;
};

/// Read `length` bytes at `offset` of the open file `file` into `out`. Return false on a
/// short read.
static bool _readAt(int file, uint64_t offset, void* out, size_t length)
{
    size_t done = 0;
    while (done < length)
    {
        const ssize_t count =
            ::pread(file, static_cast<char*>(out) + done, length - done, off_t(offset + done));
        if (count <= 0)
            return false;
        done += size_t(count);
    }
    return true;
}

/// Read the function symbols of the 64-bit ELF file at `path` into `outTable`. We prefer the
/// full symbol table, which names internal functions too, and fall back to the dynamic one.
static void _readElfFunctionTable(const char* path, ElfFunctionTable& outTable)
{
    outTable = ElfFunctionTable();
    outTable.path = path;

    const int file = ::open(path, O_RDONLY | O_CLOEXEC);
    if (file < 0)
        return;

    Elf64_Ehdr header;
    if (!_readAt(file, 0, &header, sizeof(header)) ||
        memcmp(header.e_ident, ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS64 ||
        header.e_shentsize != sizeof(Elf64_Shdr))
    {
        ::close(file);
        return;
    }

    // The loaded start of the module corresponds to the page that holds the first loadable
    // segment, so an address in the module maps to `fileBase` plus its offset.
    bool hasLoad = false;
    for (int i = 0; i < header.e_phnum; ++i)
    {
        Elf64_Phdr segment;
        if (!_readAt(
                file,
                header.e_phoff + uint64_t(i) * header.e_phentsize,
                &segment,
                sizeof(segment)))
            break;
        if (segment.p_type != PT_LOAD)
            continue;
        const uint64_t align = segment.p_align ? segment.p_align : 1;
        const uint64_t base = segment.p_vaddr & ~(align - 1);
        if (!hasLoad || base < outTable.fileBase)
            outTable.fileBase = base;
        hasLoad = true;
    }

    List<Elf64_Shdr> sections;
    sections.setCount(header.e_shnum);
    if (header.e_shnum == 0 ||
        !_readAt(file, header.e_shoff, sections.getBuffer(), sizeof(Elf64_Shdr) * header.e_shnum))
    {
        ::close(file);
        return;
    }

    const Elf64_Shdr* symbolSection = nullptr;
    for (const Elf64_Shdr& section : sections)
    {
        if (section.sh_type == SHT_SYMTAB)
            symbolSection = &section;
    }
    if (!symbolSection)
    {
        for (const Elf64_Shdr& section : sections)
        {
            if (section.sh_type == SHT_DYNSYM)
                symbolSection = &section;
        }
    }
    if (!symbolSection || symbolSection->sh_link >= sections.getCount() ||
        symbolSection->sh_entsize != sizeof(Elf64_Sym))
    {
        ::close(file);
        return;
    }
    const Elf64_Shdr& nameSection = sections[symbolSection->sh_link];

    outTable.symbols.setCount(Index(symbolSection->sh_size / sizeof(Elf64_Sym)));
    outTable.names.setCount(Index(nameSection.sh_size) + 1);
    if (_readAt(
            file,
            symbolSection->sh_offset,
            outTable.symbols.getBuffer(),
            symbolSection->sh_size) &&
        _readAt(file, nameSection.sh_offset, outTable.names.getBuffer(), nameSection.sh_size))
    {
        outTable.names[outTable.names.getCount() - 1] = 0;
        outTable.isValid = true;
    }
    ::close(file);
}

struct StackSymbolResolver::Impl
{
    ElfFunctionTable table;
};

StackSymbolResolver::StackSymbolResolver()
    : m_impl(new Impl())
{
}

StackSymbolResolver::~StackSymbolResolver()
{
    delete m_impl;
}

bool StackSymbolResolver::findFunctionName(const void* address, String& outName)
{
    Dl_info info;
    if (!::dladdr(address, &info) || !info.dli_fbase)
        return false;

    // The loader may name the main program by the path that it was started with, which no
    // longer opens if the program changed directory, so we fall back to the kernel's link.
    const char* path = info.dli_fname && info.dli_fname[0] ? info.dli_fname : "/proc/self/exe";
    if (m_impl->table.path != path)
    {
        _readElfFunctionTable(path, m_impl->table);
        if (!m_impl->table.isValid && strcmp(path, "/proc/self/exe") != 0)
        {
            _readElfFunctionTable("/proc/self/exe", m_impl->table);
            m_impl->table.path = path;
        }
    }
    if (!m_impl->table.isValid)
        return false;

    const uint64_t fileAddress =
        uint64_t(uintptr_t(address) - uintptr_t(info.dli_fbase)) + m_impl->table.fileBase;
    for (const Elf64_Sym& symbol : m_impl->table.symbols)
    {
        if (ELF64_ST_TYPE(symbol.st_info) != STT_FUNC || symbol.st_value == 0)
            continue;
        if (fileAddress < symbol.st_value || fileAddress >= symbol.st_value + symbol.st_size)
            continue;
        if (symbol.st_name >= uint64_t(m_impl->table.names.getCount()))
            return false;
        _appendDemangled(m_impl->table.names.getBuffer() + symbol.st_name, outName);
        return true;
    }
    return false;
}

#else // Apple

struct StackSymbolResolver::Impl
{
};

StackSymbolResolver::StackSymbolResolver() {}

StackSymbolResolver::~StackSymbolResolver() {}

bool StackSymbolResolver::findFunctionName(const void* address, String& outName)
{
    Dl_info info;
    if (!::dladdr(address, &info) || !info.dli_sname || !info.dli_saddr)
        return false;
    _appendDemangled(info.dli_sname, outName);
    return true;
}

#endif

/* static */ bool StackSymbolResolver::isEnabled()
{
    return true;
}

#else

// Names are not enabled in this build, or the platform has none to give.

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
