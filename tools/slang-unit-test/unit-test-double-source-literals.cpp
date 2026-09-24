// Verify the shared source writer through the public compiler API.
#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstdio>
#include <cstring>
#include <locale>
#include <sstream>

using namespace Slang;

namespace
{
// Exercise a non-classic global C++ locale without requiring installed system locales.
struct DecimalComma : std::numpunct<char>
{
    char do_decimal_point() const override { return ','; }
};

// Restore the caller's locale even when an assertion aborts this test.
struct ScopedDecimalComma
{
    std::locale previous = std::locale();
    ScopedDecimalComma() { std::locale::global(std::locale(previous, new DecimalComma)); }
    ~ScopedDecimalComma() { std::locale::global(previous); }
};
} // namespace

SLANG_UNIT_TEST(doubleSourceLiteralsRoundTrip)
{
    List<uint64_t> patterns;
    // Signed zeros, subnormals, the normal boundary, and both sides of each old/new
    // fixed/scientific boundary. Adjacent mantissas expose lost low significand bits.
    const uint64_t boundaries[] = {
        0,
        1,
        0x000fffffffffffff,
        0x0010000000000000,
        0x3ed0000000000000,
        0x3ee0000000000000,
        0x3ef0000000000000,
        0x3fe0000000000000,
        0x3ff0000000000000,
        0x40e0000000000000,
        0x40f0000000000000,
        0x4100000000000000,
        0x7fefffffffffffff,
        0x3ef0000000400000,
    };
    for (auto bits : boundaries)
    {
        for (int offset = -1; offset <= 1; ++offset)
        {
            if ((bits == 0 && offset < 0) || (bits == 0x7fefffffffffffff && offset > 0))
                continue;
            uint64_t value = bits + offset;
            patterns.add(value);
            patterns.add(value | (uint64_t(1) << 63));
        }
    }
    // Cover every finite exponent with a nontrivial mantissa, using an integer generator
    // independent of both decimal parsing and the formatter under test.
    uint64_t state = 0x123456789abcdef;
    for (uint64_t exponent = 0; exponent < 2047; ++exponent)
    {
        state = state * 6364136223846793005ull + 1442695040888963407ull;
        patterns.add((state & 0x800fffffffffffffull) | (exponent << 52));
    }

    StringBuilder source;
    source << "RWStructuredBuffer<double> doubleLiteralOutput;\n"
              "[shader(\"compute\")] [numthreads(1,1,1)] void main() {\n";
    for (Index i = 0; i < patterns.getCount(); ++i)
    {
        double value;
        memcpy(&value, &patterns[i], sizeof(value));
        char literal[128];
        // Hexadecimal input preserves the independently chosen bits exactly. The emitted
        // target literal must instead be decimal and carry those same bits.
        snprintf(literal, sizeof(literal), "%a", value);
        source << "doubleLiteralOutput[" << i << "] = " << literal << "l;\n";
    }
    source << "}\n";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    for (auto target : {SLANG_HLSL, SLANG_GLSL, SLANG_CUDA_SOURCE, SLANG_CPP_SOURCE})
    {
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createCompileRequest(request.writeRef())));
        request->setCodeGenTarget(target);
        int unit = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "doubleLiterals");
        request->addTranslationUnitSourceString(unit, "doubleLiterals.slang", source.getBuffer());
        request->addEntryPoint(unit, "main", SLANG_STAGE_COMPUTE);
        ScopedDecimalComma locale;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->compile()));
        ComPtr<ISlangBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->getEntryPointCodeBlob(0, 0, code.writeRef())));
        const char* cursor = static_cast<const char*>(code->getBufferPointer());
        for (auto expected : patterns)
        {
            // Only stores to our uniquely named output are inspected; target boilerplate
            // and resource declarations cannot be mistaken for literals.
            for (;;)
            {
                cursor = strstr(cursor, "doubleLiteralOutput");
                SLANG_CHECK_ABORT(cursor != nullptr);
                const char* end = strchr(cursor, '\n');
                const char* assignment = strstr(cursor, " = ");
                ++cursor;
                if (assignment && (!end || assignment < end))
                {
                    cursor = assignment + 3;
                    break;
                }
            }
            std::istringstream parser(cursor);
            parser.imbue(std::locale::classic());
            double actual = 0;
            parser >> actual;
            SLANG_CHECK_ABORT(!parser.fail());
            uint64_t actualBits;
            memcpy(&actualBits, &actual, sizeof(actualBits));
            SLANG_CHECK(actualBits == expected);
            const char* end = strchr(cursor, ';');
            SLANG_CHECK_ABORT(end != nullptr);
            bool hasDecimalPoint = false;
            for (const char* c = cursor; c != end; ++c)
            {
                SLANG_CHECK(*c != ',');
                hasDecimalPoint |= *c == '.';
            }
            SLANG_CHECK(hasDecimalPoint);
            cursor = end + 1;
        }
        SLANG_CHECK(std::use_facet<std::numpunct<char>>(std::locale()).decimal_point() == ',');
    }
}
