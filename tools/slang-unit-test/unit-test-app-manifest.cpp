// unit-test-app-manifest.cpp
//
// Guards for the shared longPathAware application manifest (cmake/slang-app.manifest):
// the manifest content and the CMake wiring that attaches it (every platform the
// unit tests run on — Linux CI never reads the file otherwise), and, on Windows,
// the manifest actually embedded in the running executable.

#include "core/slang-io.h"
#include "unit-test/slang-unit-test.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#endif

using namespace Slang;

// The shared longPathAware application manifest (cmake/slang-app.manifest) is
// only attached to targets under WIN32 AND MSVC, so no compile or link step on
// Linux ever reads the file, and an edit that drops the longPathAware setting
// would otherwise stay green on every Linux CI leg. Reading the manifest
// directly from the source tree here keeps that drift visible on every
// platform the unit tests run on.
SLANG_UNIT_TEST(appManifestIsDeclaredAndWired)
{
    const char* manifestPath = "cmake/slang-app.manifest";
    SLANG_CHECK_ABORT(File::exists(manifestPath));

    String manifest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(manifestPath, manifest)));
    SLANG_CHECK_ABORT(manifest.contains("<longPathAware>true</longPathAware>"));

    // The load-bearing wiring is the call slang_add_target makes for every
    // executable it creates, and the WIN32 AND MSVC guard inside the helper;
    // a bare mention of the manifest file would pass on a comment or a dead
    // call, so anchor on the call itself.
    String slangTarget;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText("cmake/SlangTarget.cmake", slangTarget)));
    SLANG_CHECK_ABORT(slangTarget.contains("slang_add_windows_manifest(${target})"));
    SLANG_CHECK_ABORT(slangTarget.contains("if(WIN32 AND MSVC)"));
    SLANG_CHECK_ABORT(slangTarget.contains("slang-app.manifest"));

    // Executables declared with a raw add_executable opt in by calling the
    // helper themselves; guard the one such call site in the main build.
    String exampleCMake;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText("examples/shader-coverage-backends/CMakeLists.txt", exampleCMake)));
    SLANG_CHECK_ABORT(
        exampleCMake.contains("slang_add_windows_manifest(shader-coverage-backends)"));
}

#if SLANG_WINDOWS_FAMILY
// slang-test, which hosts and runs these unit tests, is created through
// slang_add_target, so it carries the shared longPathAware manifest. Finding
// the setting in the running executable's own RT_MANIFEST resource proves the
// embed pipeline end to end — the manifest source made it through CMake and
// the manifest tool into the binary — without needing a long-path-enabled
// machine. If this fails, file I/O silently breaks again for paths over
// MAX_PATH on long-path-enabled machines.
//
// The embed itself only happens under WIN32 AND MSVC, while this test compiles
// for any Windows toolchain, so a future non-MSVC Windows build would fail
// here — deliberately, since such a build would otherwise silently lack
// long-path support.
SLANG_UNIT_TEST(embeddedAppManifestDeclaresLongPathAware)
{
    HMODULE module = GetModuleHandleW(nullptr);
    SLANG_CHECK_ABORT(module != nullptr);

    HRSRC resource = FindResourceW(module, CREATEPROCESS_MANIFEST_RESOURCE_ID, RT_MANIFEST);
    SLANG_CHECK_ABORT(resource != nullptr);

    HGLOBAL loaded = LoadResource(module, resource);
    SLANG_CHECK_ABORT(loaded != nullptr);

    const char* text = (const char*)LockResource(loaded);
    SLANG_CHECK_ABORT(text != nullptr);

    // The resource is a byte range, not a string: it is not guaranteed to be
    // NUL-terminated, so copy exactly its length before searching.
    DWORD size = SizeofResource(module, resource);
    SLANG_CHECK_ABORT(size != 0);

    String manifest(text, text + size);
    SLANG_CHECK_ABORT(manifest.contains("<longPathAware>true</longPathAware>"));
}
#endif
