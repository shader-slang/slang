// Link check for the bundled static archive produced by SLANG_BUNDLE_STATIC_LIB.
//
// This is compiled against nothing but the staged release tree -- its include/
// directory and the single merged archive in its lib/ directory. No CMake, no
// slang::slang target, no list of internal archives. That is the whole point of
// the bundle: a consumer that has never seen Slang's build system links one
// file.
//
// Creating and releasing a global session is enough to prove the archive is
// well formed. It resolves symbols across core, compiler-core and the core
// module, which live in different members of the merged archive, and it runs
// the static initializers of every bundled library. A malformed merge -- the
// real risk on the macOS libtool and Windows lib.exe paths, neither of which
// had been exercised before this workflow existed -- fails here at link time
// rather than silently shipping.
//
// The embedded glslang/SPIRV-Tools path is covered separately, by the slangc
// invocations in release-static.yml.

#include <cstdio>
#include <slang.h>

int main()
{
    slang::IGlobalSession* globalSession = nullptr;

    const SlangResult result = slang_createGlobalSession(SLANG_API_VERSION, &globalSession);
    if (SLANG_FAILED(result))
    {
        std::fprintf(
            stderr,
            "slang_createGlobalSession failed: 0x%08x\n",
            static_cast<unsigned int>(result));
        return 1;
    }
    if (globalSession == nullptr)
    {
        std::fprintf(stderr, "slang_createGlobalSession succeeded but returned null\n");
        return 1;
    }

    globalSession->release();

    std::printf("static consumer link check OK\n");
    return 0;
}
