#include "slang-test/test-output-path-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static List<String> makeArgs(const char* const* values, Index count)
{
    List<String> args;
    for (Index i = 0; i < count; ++i)
        args.add(values[i]);
    return args;
}

static void checkArgs(const List<String>& args, const char* const* expected, Index expectedCount)
{
    SLANG_CHECK_ABORT(args.getCount() == expectedCount);
    for (Index i = 0; i < expectedCount; ++i)
        SLANG_CHECK(args[i] == expected[i]);
}

SLANG_UNIT_TEST(slangTestOutputPathNormalization)
{
    const String testPath = "tests/diagnostics/path-normalization.slang";

    {
        const char* values[] = {"-target", "spirv", "-o", "out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {"-target", "spirv", "-o", "tests/diagnostics/out.spv"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {
            "-target",
            "spirv",
            "-separate-debug-info-output",
            "debug.spv",
        };
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {
            "-target",
            "spirv",
            "-separate-debug-info-output",
            "tests/diagnostics/debug.spv",
        };
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-separate-debug-info-output", "-"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {"-target", "spirv", "-separate-debug-info-output", "-"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "-"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {"-target", "spirv", "-o", "-"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    // A POSIX-absolute `-o` path is rejected, and the error names both the option and the path so a
    // test author can see what to change.
    {
        const char* values[] = {"-target", "spirv", "-o", "/tmp/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
        SLANG_CHECK(error.indexOf(UnownedStringSlice("-o")) >= 0);
        SLANG_CHECK(error.indexOf(UnownedStringSlice("/tmp/out.spv")) >= 0);
    }

    // The null device gets no exemption on any host: `/dev/null` is an absolute path like any
    // other, and it is not a path a Windows compiler can open. A test that wants to discard its
    // output uses `-o -` instead, which the case above already covers.
    {
        const char* values[] = {"-target", "spirv-asm", "-o", "/dev/null"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    {
        const char* values[] = {"-separate-debug-info-output", "/tmp/debug.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    // A rooted Windows drive path is rejected on every host, so a directive authored on Windows is
    // caught by Linux CI too.
    {
        const char* values[] = {"-o", "C:\\out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    // A quoted absolute path must also be rejected: slang-test unescapes arguments shell-style
    // before running them, so the guard classifies the unescaped form rather than the raw token
    // whose leading character is a quote.
    {
        const char* values[] = {"-o", "\"/tmp/out.spv\""};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    // Malformed quoting (an unterminated quote) must fail loudly rather than unescape to an empty
    // string that would read as non-absolute and bypass the check.
    {
        const char* values[] = {"-o", "\"/tmp/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "nested/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {"-target", "spirv", "-o", "nested/out.spv"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "../leak.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        checkArgs(args, values, SLANG_COUNT_OF(values));
    }

    // A forward-slash rooted drive (`C:/`) is absolute too, so it is rejected like `C:\`.
    {
        const char* values[] = {"-o", "C:/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_FAILED(normalizeTestOutputPathsForTestFile(testPath, args, error)));
    }

    // A quoted relative path is accepted; classification uses the unescaped form, while
    // normalization anchors the stored (still-quoted) bare value beside the test file — matching
    // the pre-existing behaviour that operates on the raw argument (slang-test unescapes again at
    // run time).
    {
        const char* values[] = {"-o", "\"out.spv\""};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {"-o", "tests/diagnostics/\"out.spv\""};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-o", "a.dxbc", "-o", "b.dxbc"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        const char* expected[] = {
            "-o",
            "tests/diagnostics/a.dxbc",
            "-o",
            "tests/diagnostics/b.dxbc",
        };
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        String error;
        SLANG_CHECK(SLANG_SUCCEEDED(normalizeTestOutputPathsForTestFile(testPath, args, error)));

        checkArgs(args, values, SLANG_COUNT_OF(values));
    }
}
