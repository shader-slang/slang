// slang-stdin-source.h
#ifndef SLANG_STDIN_SOURCE_H
#define SLANG_STDIN_SOURCE_H

#include "../core/slang-basic.h"

#include <stdio.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace Slang
{

enum class StdinSourceReadResult
{
    Success,
    CannotRead,
    TooLarge,
};

inline StdinSourceReadResult readStdinSource(FILE* input, Index maxBytes, List<Byte>& outSource)
{
    outSource.clear();

    if (!input)
        return StdinSourceReadResult::CannotRead;

#ifdef _WIN32
    const int previousMode = _setmode(_fileno(input), _O_BINARY);
    struct StdinModeGuard
    {
        FILE* input;
        int mode;
        ~StdinModeGuard()
        {
            if (mode != -1)
                _setmode(_fileno(input), mode);
        }
    } stdinModeGuard{input, previousMode};
#endif

    clearerr(input);

    // Use blocking stdio here. The process PipeStream path is optimized for polling child
    // process output and can spin while waiting for piped stdin.
    char buffer[16 * 1024];
    for (;;)
    {
        const size_t readByteCount = fread(buffer, 1, sizeof(buffer), input);
        if (readByteCount != 0)
        {
            const Index readCount = Index(readByteCount);
            if (outSource.getCount() > maxBytes || readCount > maxBytes - outSource.getCount())
            {
                outSource.clear();
                return StdinSourceReadResult::TooLarge;
            }
            outSource.addRange(reinterpret_cast<const Byte*>(buffer), readCount);
            continue;
        }

        if (ferror(input))
        {
            outSource.clear();
            return StdinSourceReadResult::CannotRead;
        }

        return StdinSourceReadResult::Success;
    }
}

} // namespace Slang

#endif
