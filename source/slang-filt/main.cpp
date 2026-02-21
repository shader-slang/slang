// slang-filt main.cpp

#include "../slang/slang-demangle.h"

#include "../core/slang-io.h"
#include "../core/slang-std-writers.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace Slang;

static void printUsage(const char* exeName)
{
    auto out = StdWriters::getOut();
    out.print("Usage: %s [symbol...]\n", exeName ? exeName : "slang-filt");
    out.put("Demangles Slang mangled symbol names.\n");
    out.put("When no symbols are provided, slang-filt reads whitespace-separated symbols from stdin.\n");
}

static void writeDemangled(const UnownedStringSlice& slice)
{
    if (!slice.getLength())
        return;
    String demangled = demangleName(slice);
    StdWriters::getOut().print("%s\n", demangled.getBuffer());
}

static bool isHelpFlag(const UnownedStringSlice& arg)
{
    return arg == "-h" || arg == "--help" || arg == "-help";
}

static int run(int argc, const char* const* argv)
{
    const char* exeName = (argc > 0 && argv) ? argv[0] : "slang-filt";
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            UnownedStringSlice arg(argv[i]);
            if (isHelpFlag(arg))
            {
                printUsage(exeName);
                return 0;
            }
        }

        for (int i = 1; i < argc; ++i)
        {
            writeDemangled(UnownedStringSlice(argv[i]));
        }
        return 0;
    }

    std::string line;
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        std::string token;
        while (iss >> token)
        {
            writeDemangled(UnownedStringSlice(token.c_str(), token.size()));
        }
    }
    return 0;
}

#ifdef _WIN32
#define MAIN slangfilt_main
#else
#define MAIN main
#endif

int MAIN(int argc, char** argv)
{
    auto stdWriters = StdWriters::initDefaultSingleton();
    SLANG_UNUSED(stdWriters);
    return run(argc, const_cast<const char* const*>(argv));
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    List<String> args;
    for (int i = 0; i < argc; ++i)
    {
        args.add(String::fromWString(argv[i]));
    }
    List<const char*> argBuffers;
    for (auto& arg : args)
    {
        argBuffers.add(arg.getBuffer());
    }
    return MAIN(argc, (char**)argBuffers.getBuffer());
}
#endif
