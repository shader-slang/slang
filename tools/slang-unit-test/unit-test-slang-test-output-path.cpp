#include "../slang-test/test-output-path-util.h"
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
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {"-target", "spirv", "-o", "tests/diagnostics/out.spv"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "-"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {"-target", "spirv", "-o", "-"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "/tmp/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {"-target", "spirv", "-o", "/tmp/out.spv"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-o", "nested/out.spv"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {"-target", "spirv", "-o", "nested/out.spv"};
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {"-target", "spirv", "-dump-intermediates"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {
            "-target",
            "spirv",
            "-dump-intermediates",
            "-dump-intermediate-prefix",
            "tests/diagnostics/path-normalization-",
        };
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {
            "-target",
            "spirv",
            "-dump-intermediates",
            "-dump-intermediate-prefix",
            "explicit-",
        };
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {
            "-target",
            "spirv",
            "-dump-intermediates",
            "-dump-intermediate-prefix",
            "tests/diagnostics/explicit-",
        };
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }

    {
        const char* values[] = {
            "-target",
            "spirv",
            "-dump-intermediates",
            "-dump-intermediate-prefix",
            "nested/explicit-",
        };
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        checkArgs(args, values, SLANG_COUNT_OF(values));
    }

    {
        const char* values[] = {"-o", "a.dxbc", "-o", "b.dxbc"};
        List<String> args = makeArgs(values, SLANG_COUNT_OF(values));
        normalizeTestOutputPathsForTestFile(testPath, args);

        const char* expected[] = {
            "-o",
            "tests/diagnostics/a.dxbc",
            "-o",
            "tests/diagnostics/b.dxbc",
        };
        checkArgs(args, expected, SLANG_COUNT_OF(expected));
    }
}
