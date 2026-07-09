// unit-test-path.cpp

#include "../../source/core/slang-io.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(path)
{
#if SLANG_WINDOWS_FAMILY
    // Disable for now on non windows has some problems on *some* Linux based CI.
    {
        String path;
        SlangResult res = Path::getCanonical("source/slang", path);
        SLANG_CHECK(SLANG_SUCCEEDED(res));

        String parentPath;
        res = Path::getCanonical("source", parentPath);
        SLANG_CHECK(SLANG_SUCCEEDED(res));

        String parentPath2 = Path::getParentDirectory(path);
        SLANG_CHECK(parentPath == parentPath2);
    }
#endif
    // Test the paths
    {
        SLANG_CHECK(Path::simplify(".") == ".");
        SLANG_CHECK(Path::simplify("..") == "..");
        SLANG_CHECK(Path::simplify("blah/..") == ".");

        SLANG_CHECK(Path::simplify("blah/.././a") == "a");

        SLANG_CHECK(Path::simplify("a:/what/.././../is/./../this/.") == "a:/../this");

        SLANG_CHECK(Path::simplify("a:/what/.././../is/./../this/./") == "a:/../this");

        SLANG_CHECK(Path::simplify("a:\\what\\..\\.\\..\\is\\.\\..\\this\\.\\") == "a:/../this");

        SLANG_CHECK(
            Path::simplify("tests/preprocessor/.\\pragma-once-a.h") ==
            "tests/preprocessor/pragma-once-a.h");


        SLANG_CHECK(Path::hasRelativeElement("."));
        SLANG_CHECK(Path::hasRelativeElement(".."));
        SLANG_CHECK(Path::hasRelativeElement("blah/.."));

        SLANG_CHECK(Path::hasRelativeElement("blah/.././a"));
        SLANG_CHECK(Path::hasRelativeElement("a") == false);
        SLANG_CHECK(Path::hasRelativeElement("blah/a") == false);
        SLANG_CHECK(Path::hasRelativeElement("a:\\blah/a") == false);


        SLANG_CHECK(Path::hasRelativeElement("a:/what/.././../is/./../this/."));

        SLANG_CHECK(Path::hasRelativeElement("a:/what/.././../is/./../this/./"));

        SLANG_CHECK(Path::hasRelativeElement("a:\\what\\..\\.\\..\\is\\.\\..\\this\\.\\"));
    }

    // Regression test for shader-slang/slang#11918. `getRelativePath` must never return an empty
    // (unresolvable) path for valid, non-empty inputs. `std::filesystem::relative` returns an empty
    // path when the inputs have different root names -- notably distinct Windows drive letters
    // (`C:\...` vs `D:\...`). The serialized-module up-to-date check stored that empty string as a
    // module's own source dependency, so a cached `.slang-module` on a different drive than its
    // source was always rejected as stale. The fix falls back to the original absolute path, which
    // the include system can still resolve via its absolute-path branch.
    {
        // The non-empty guarantee holds on every platform (on Windows the inputs are cross-drive so
        // no relative path exists and the fix returns the absolute input; on other platforms the
        // backslash strings are ordinary same-root relative paths). Without the fix this is empty
        // on Windows.
        const String crossRoot = Path::getRelativePath("D:\\cache", "C:\\src\\module.slang");
        SLANG_CHECK(crossRoot.getLength() > 0);
#if SLANG_WINDOWS_FAMILY
        // On Windows the two inputs are on different drives, so the fallback returns the original
        // absolute path verbatim, which resolves independently of any "from" directory.
        SLANG_CHECK(crossRoot == "C:\\src\\module.slang");
        SLANG_CHECK(Path::isAbsolute(crossRoot.getUnownedSlice()));

        // Same-drive inputs still yield a normal relative path; the fix does not disturb this.
        const String sameDrive = Path::getRelativePath("C:\\src", "C:\\src\\module.slang");
        SLANG_CHECK(sameDrive == "module.slang");
#endif
    }

    // hasPath: returns true only when the path contains at least one separator.
    // These cases matter for _getCanonicalOrExecutablePath: bare names must
    // fall through to the OS-fallback branch even if realpath() would succeed
    // against cwd.
    {
        // Bare names — no separator, no path context.
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("slang-test")) == false);
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("slang-test.exe")) == false);
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("")) == false);

        // Names with explicit path context.
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("./slang-test")));
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("/usr/bin/slang-test")));
        SLANG_CHECK(Path::hasPath(UnownedStringSlice("D:\\foo\\slang-test.exe")));
    }
}
