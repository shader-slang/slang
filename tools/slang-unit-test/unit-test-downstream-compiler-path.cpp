#include "core/slang-platform.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Recovering the loaded compiler's path is a pure metadata query, so this runs without a GPU: it
// only needs the downstream shared library to load, not a device. Availability of any given
// pass-through varies by machine, so each case below is gated the same way the codegen tests gate
// their pass-throughs.
SLANG_UNIT_TEST(getDownstreamCompilerPath)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    // NONE is not a real compiler, so it is rejected before any discovery.
    {
        ComPtr<ISlangBlob> path;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NONE, path.writeRef()) ==
            SLANG_E_NOT_FOUND);
    }

    // An out-of-range pass-through value must be rejected at the boundary, not used to index the
    // per-type compiler arrays.
    {
        ComPtr<ISlangBlob> path;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(
                SlangPassThrough(SLANG_PASS_THROUGH_COUNT_OF),
                path.writeRef()) == SLANG_E_NOT_FOUND);
    }

    // Gate the NVRTC path on availability, so this test passes both on machines that have NVRTC and
    // those that do not (no GPU is required merely to load the NVRTC library and recover its path).
    const bool nvrtcAvailable =
        SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC));

    ComPtr<ISlangBlob> nvrtcPath;
    const SlangResult result =
        globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NVRTC, nvrtcPath.writeRef());

    if (nvrtcAvailable)
    {
        // NVRTC is a shared library, so its path is recoverable from a held symbol.
        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK_ABORT(nvrtcPath != nullptr); // abort rather than deref null below on regression
        SLANG_CHECK(nvrtcPath->getBufferSize() > 0);

        // The central contract is that this is the exact loadable library Slang uses, not merely a
        // non-empty string. Prove it: reload the returned path by its platform path, resolve the
        // nvrtcVersion entry point, and call it. A path pointing at the wrong library (or a stale
        // string) would fail to load or would not expose nvrtcVersion; a real NVRTC loads and
        // reports a plausible, non-zero major version.
        const char* nvrtcPathStr = static_cast<const char*>(nvrtcPath->getBufferPointer());
        SharedLibrary::Handle handle = nullptr;
        SLANG_CHECK(SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(nvrtcPathStr, handle)));
        if (handle)
        {
            // nvrtcVersion(int* major, int* minor) returns nvrtcResult (0 == NVRTC_SUCCESS).
            using NvrtcVersionFunc = int (*)(int*, int*);
            auto nvrtcVersion = reinterpret_cast<NvrtcVersionFunc>(
                SharedLibrary::findSymbolAddressByName(handle, "nvrtcVersion"));
            SLANG_CHECK(nvrtcVersion != nullptr);
            if (nvrtcVersion)
            {
                int libMajor = 0;
                int libMinor = 0;
                SLANG_CHECK(nvrtcVersion(&libMajor, &libMinor) == 0);
                SLANG_CHECK(libMajor > 0);
            }
            SharedLibrary::unload(handle);
        }
    }
    else
    {
        SLANG_CHECK(result == SLANG_E_NOT_FOUND);
    }

    // glslang is a bundled, GPU-independent shared library that loads on CI runners without a
    // device. When loadable, its path must be recoverable (SLANG_OK + non-empty), exercising the
    // shared-library recovery path for a compiler that reports no numeric version. This also gives
    // the "reload the path and verify an entry point" check runtime coverage on a GPU-free host,
    // which the NVRTC block above cannot: reload the returned path and confirm a glslang compile
    // entry point resolves from it, proving it is the real loadable library and not just a string.
    if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_GLSLANG)))
    {
        ComPtr<ISlangBlob> glslangPath;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(
                SLANG_PASS_THROUGH_GLSLANG,
                glslangPath.writeRef()) == SLANG_OK);
        SLANG_CHECK_ABORT(
            glslangPath != nullptr); // abort rather than deref null below on regression
        SLANG_CHECK(glslangPath->getBufferSize() > 0);

        const char* glslangPathStr = static_cast<const char*>(glslangPath->getBufferPointer());
        SharedLibrary::Handle handle = nullptr;
        SLANG_CHECK(SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(glslangPathStr, handle)));
        if (handle)
        {
            // The glslang downstream binds to whichever compile entry point the library exposes,
            // trying glslang_compile then the versioned glslang_compile_1_1/_1_2/_1_3
            // (slang-glslang-compiler.cpp init). At least one must resolve from the returned
            // library; requiring the unversioned name alone would wrongly reject a versioned-only
            // build, so accept any of them.
            const bool anyCompileSymbol =
                SharedLibrary::findSymbolAddressByName(handle, "glslang_compile") != nullptr ||
                SharedLibrary::findSymbolAddressByName(handle, "glslang_compile_1_1") != nullptr ||
                SharedLibrary::findSymbolAddressByName(handle, "glslang_compile_1_2") != nullptr ||
                SharedLibrary::findSymbolAddressByName(handle, "glslang_compile_1_3") != nullptr;
            SLANG_CHECK(anyCompileSymbol);
            SharedLibrary::unload(handle);
        }
    }

    // Exercise the third return code, SLANG_E_NOT_AVAILABLE -- the "loaded but no recoverable path"
    // case the client must keep distinct from SLANG_E_NOT_FOUND. GCC/Clang/Visual Studio are backed
    // by executables on PATH: each derives CommandLineDownstreamCompiler -> DownstreamCompilerBase
    // and does not override getPath, so once located it hits the base default and reports no
    // recoverable path. This is reachable GPU-free wherever a host C/C++ toolchain exists
    // (g++/clang on POSIX, cl on Windows), and is gated on availability like the NVRTC/glslang
    // cases above. (We use the concrete executable pass-throughs rather than the GENERIC_C_CPP
    // alias, whose default C/C++ compiler can resolve to the shared-library-backed slang-llvm,
    // which *does* have a recoverable path and would return SLANG_OK.)
    for (auto cppPassThrough :
         {SLANG_PASS_THROUGH_GCC, SLANG_PASS_THROUGH_CLANG, SLANG_PASS_THROUGH_VISUAL_STUDIO})
    {
        if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(cppPassThrough)))
        {
            // Seed a non-null sentinel and require it to survive the call: this proves the failure
            // return leaves *outPath untouched. A null starting value could not distinguish an
            // untouched pointer from one set to null. The sentinel is never dereferenced.
            ISlangBlob* const sentinel =
                reinterpret_cast<ISlangBlob*>(static_cast<uintptr_t>(0xF00DF00DF00DF00Dull));
            ISlangBlob* cppPath = sentinel;
            SLANG_CHECK(
                globalSession->getDownstreamCompilerPath(cppPassThrough, &cppPath) ==
                SLANG_E_NOT_AVAILABLE);
            SLANG_CHECK(cppPath == sentinel);
        }
    }
}
