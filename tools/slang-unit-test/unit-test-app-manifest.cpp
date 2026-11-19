// unit-test-app-manifest.cpp
//
// Guards for the shared longPathAware application manifest
// (cmake/slang-app.manifest). On every platform the unit tests run on: the
// manifest content, and the CMake wiring that attaches it (Linux builds never
// read the file, so drift would otherwise stay green on every Linux CI leg).
// On Windows additionally: the manifest actually embedded in the running
// executable's RT_MANIFEST resource.
//
// The tests read repo files by paths relative to the working directory; like
// the other repo-tree-reading unit tests, they assume slang-test runs from
// the repository root.

#include "core/slang-io.h"
#include "unit-test/slang-unit-test.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#endif

using namespace Slang;

// Returns the text between the first occurrence of beginMarker and the next
// endMarker after it, or an empty slice if either marker is missing.
static UnownedStringSlice _sliceBetween(
    const String& text,
    const char* beginMarker,
    const char* endMarker)
{
    const UnownedStringSlice begin(beginMarker);
    const UnownedStringSlice end(endMarker);

    const Index beginAt = text.indexOf(begin);
    if (beginAt < 0)
        return UnownedStringSlice();

    const Index bodyBegin = beginAt + begin.getLength();
    UnownedStringSlice tail(text.begin() + bodyBegin, text.end());
    const Index endAt = tail.indexOf(end);
    if (endAt < 0)
        return UnownedStringSlice();

    return tail.subString(0, endAt);
}

static bool _contains(const UnownedStringSlice& slice, const char* needle)
{
    return slice.indexOf(UnownedStringSlice(needle)) >= 0;
}

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
    SLANG_CHECK_ABORT(manifest.contains("<ws2:longPathAware>true</ws2:longPathAware>"));

    String slangTarget;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText("cmake/SlangTarget.cmake", slangTarget)));

    // The wiring checks are scoped to the function bodies they pin, so an
    // unrelated WIN32 AND MSVC branch or a comment mentioning the manifest
    // cannot satisfy them, and a benign rename inside an unrelated function
    // cannot break them.
    //
    // The load-bearing call is the one slang_add_target makes for every
    // executable it creates.
    const UnownedStringSlice addTargetBody =
        _sliceBetween(slangTarget, "function(slang_add_target dir type)", "endfunction()");
    SLANG_CHECK_ABORT(_contains(addTargetBody, "slang_add_windows_manifest(${target})"));

    // The helper itself must still gate on the platform and attach the shared
    // manifest.
    const UnownedStringSlice helperBody =
        _sliceBetween(slangTarget, "function(slang_add_windows_manifest target)", "endfunction()");
    SLANG_CHECK_ABORT(_contains(helperBody, "if(WIN32 AND MSVC)"));
    SLANG_CHECK_ABORT(_contains(helperBody, "slang-app.manifest"));

    // Executables declared with a raw add_executable opt in by calling the
    // helper themselves; guard the one such call site in the main build.
    String exampleCMake;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText("examples/shader-coverage-backends/CMakeLists.txt", exampleCMake)));
    SLANG_CHECK_ABORT(
        exampleCMake.contains("slang_add_windows_manifest(shader-coverage-backends)"));
}

#if SLANG_WINDOWS_FAMILY
// The unit tests run in-process inside the executable that hosts them —
// slang-test, or test-server when slang-test runs under -use-test-server —
// both of which are created through slang_add_target and so carry the shared
// longPathAware manifest. Finding the setting in the running executable's
// own RT_MANIFEST resource proves the embed pipeline end to end — the
// manifest source made it through CMake and the manifest tool into the
// binary — without needing a long-path-enabled machine. If this fails, file
// I/O silently breaks again for paths over MAX_PATH on long-path-enabled
// machines.
//
// The embed itself only happens under WIN32 AND MSVC, while this test
// compiles for any Windows toolchain, so a future non-MSVC Windows build
// would fail here — deliberately, since such a build would otherwise
// silently lack long-path support.
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
    SLANG_CHECK_ABORT(manifest.contains("<ws2:longPathAware>true</ws2:longPathAware>"));
}
#endif
