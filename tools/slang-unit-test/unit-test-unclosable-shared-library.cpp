// unit-test-unclosable-shared-library.cpp

#include "core/slang-platform.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// `slang-llvm` must never be unmapped once it has been loaded, because the allocator runtimes
/// statically linked into it (LLVM's vendored rpmalloc, and mimalloc) register a thread-exit
/// destructor that the OS still invokes during process shutdown, long after any module has been
/// unloaded. Unmapping the library leaves that callback dangling and turns a normal process exit
/// into an execute access violation -- see issue #12292, where
/// `tests/cpu-program/gfx-smoke.slang` produced all of its expected output and then died with
/// `0xC0000005` inside `ntdll!RtlpFlsDataCleanup`.
///
/// `SharedLibrary::loadWithPlatformPath` keeps every library that `SharedLibrary::isUnclosable`
/// reports resident, so this checks that predicate. The teardown crash itself only reproduces in
/// a build that links slang-llvm against a system LLVM, whereas this holds in every build flavour.
SLANG_UNIT_TEST(unclosableSharedLibrary)
{
    // slang-llvm is unclosable under each platform's decoration of the name, and whether it is
    // named on its own or at the end of a path.
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("slang-llvm.dll")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("libslang-llvm.so")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("libslang-llvm.dylib")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("C:\\slang\\bin\\slang-llvm.dll")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("/opt/slang/bin/libslang-llvm.so")));

#if SLANG_WINDOWS_FAMILY
    // Windows file names are case-insensitive, so these name the same file as `slang-llvm.dll`
    // and must be recognized as well. The POSIX platforms treat them as different files, so the
    // matching there stays case-sensitive and these are deliberately not checked.
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("SLANG-LLVM.DLL")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("Slang-LLVM.Dll")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("C:\\slang\\bin\\SLANG-LLVM.DLL")));
#endif

    // The other entries carry their own workarounds and must stay in the list.
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("libdxcompiler.so")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("libdxvk_d3d11.so")));
    SLANG_CHECK(SharedLibrary::isUnclosable(toSlice("libdxvk_dxgi.so")));

    // Everything else stays closable. In particular a directory that happens to be named after an
    // unclosable library must not make an unrelated library unclosable, and neither must a library
    // whose name merely ends with one.
    SLANG_CHECK(!SharedLibrary::isUnclosable(toSlice("")));
    SLANG_CHECK(!SharedLibrary::isUnclosable(toSlice("slang-glslang.dll")));
    SLANG_CHECK(!SharedLibrary::isUnclosable(toSlice("/opt/slang-llvm/lib/libfoo.so")));
    SLANG_CHECK(!SharedLibrary::isUnclosable(toSlice("not-slang-llvm.dll")));
}
