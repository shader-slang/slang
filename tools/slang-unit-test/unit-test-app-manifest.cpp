// unit-test-app-manifest.cpp
//
// Guard for the shared longPathAware application manifest
// (cmake/slang-app.manifest): on Windows, the manifest actually embedded in
// the running executable's RT_MANIFEST resource.

#include "unit-test/slang-unit-test.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#endif

using namespace Slang;

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
SLANG_UNIT_TEST(embeddedAppManifestDeclaresLongPathAware, NoRenderApi)
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
