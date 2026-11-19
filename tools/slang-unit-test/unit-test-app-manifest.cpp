// unit-test-app-manifest.cpp

#include "core/slang-io.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// The shared longPathAware application manifest (cmake/slang-app.manifest) is
// only attached to targets under WIN32 AND MSVC, so no compile or link step on
// Linux ever reads the file, and an edit that drops the longPathAware setting
// would otherwise stay green on every Linux CI leg. Reading the manifest
// directly from the source tree here keeps that drift visible on every
// platform the unit tests run on.
SLANG_UNIT_TEST(appManifestDeclaresLongPathAware)
{
    const char* manifestPath = "cmake/slang-app.manifest";
    SLANG_CHECK_ABORT(File::exists(manifestPath));

    String manifest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(manifestPath, manifest)));
    SLANG_CHECK_ABORT(manifest.contains("<longPathAware>true</longPathAware>"));
}
