// slang-core-module-cache.cpp

#include "core/slang-builtin-module-cache.h"
#include "core/slang-io.h"
#include "core/slang-shared-library.h"

#include <stdio.h>

using namespace Slang;

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        fprintf(
            stderr,
            "Adds a shared-library timestamp to Slang's core-module archive.\n"
            "\n"
            "When the core module is not embedded, its build flow has four steps:\n"
            "\n"
            "  1. Run slang-generate on source/slang/*.meta.slang to produce C++ headers that\n"
            "     construct the core module's Slang source text.\n"
            "  2. Build the Slang shared library, such as slang-compiler.dll, with that generated\n"
            "     source text embedded in it. This is source, not a compiled module archive.\n"
            "  3. Run slang-bootstrap with the new shared library. It compiles the embedded "
            "source\n"
            "     text into the intermediate archive slang-core-module-without-timestamp.bin.\n"
            "  4. Optionally run this tool to prefix that archive with the shared library's\n"
            "     modification timestamp and write the runtime cache slang-core-module.bin beside\n"
            "     the library.\n"
            "\n"
            "The archive without a timestamp is an intermediate serialized module that build-time\n"
            "compiler invocations can load explicitly. The cache output contains the same archive\n"
            "bytes, but its timestamp prefix binds it to one shared-library build. At startup,\n"
            "Slang checks that timestamp before loading the cache.\n"
            "\n"
            "Step 4 is an optimization, not a requirement. If the runtime cache is absent, the\n"
            "first slangc session compiles the source text embedded in the shared library and\n"
            "writes the cache itself; it does not load the archive without a timestamp\n"
            "automatically.\n"
            "\n"
            "Running this tool during the build packages the archive already produced in step 3\n"
            "and avoids that second core-module compilation on first use. It only performs\n"
            "step 4; it does not compile the module.\n"
            "\n"
            "The cache output layout is:\n"
            "\n"
            "    [uint64 shared-library timestamp][module archive without timestamp]\n"
            "\n"
            "arguments:\n"
            "  <shared-library>  Final Slang shared library whose timestamp validates the cache.\n"
            "  <archive-without-timestamp>\n"
            "                    Serialized core module produced in step 3.\n"
            "  <cache-output>    Timestamp-prefixed file consumed by Slang at runtime.\n"
            "\n");
        fprintf(
            stderr,
            "usage: %s <shared-library> <archive-without-timestamp> <cache-output>\n",
            argv[0]);
        fprintf(
            stderr,
            "example: %s build/Debug/bin/slang-compiler.dll "
            "build/source/slang-core-module/slang-core-module-without-timestamp.bin "
            "build/Debug/bin/slang-core-module.bin\n",
            argv[0]);
        return 1;
    }

    const String libraryPath = argv[1];
    const String archivePath = argv[2];
    const String cachePath = argv[3];

    const uint64_t libraryTimestamp = SharedLibraryUtils::getFileTimestamp(libraryPath);
    if (libraryTimestamp == 0)
    {
        fprintf(stderr, "unable to read shared-library timestamp: %s\n", libraryPath.getBuffer());
        return 1;
    }

    ScopedAllocation archive;
    if (SLANG_FAILED(File::readAllBytes(archivePath, archive)))
    {
        fprintf(stderr, "unable to read module archive: %s\n", archivePath.getBuffer());
        return 1;
    }

    if (SLANG_FAILED(BuiltinModuleCache::write(
            cachePath,
            libraryTimestamp,
            archive.getData(),
            archive.getSizeInBytes())))
    {
        fprintf(stderr, "unable to write module cache: %s\n", cachePath.getBuffer());
        return 1;
    }
    return 0;
}
