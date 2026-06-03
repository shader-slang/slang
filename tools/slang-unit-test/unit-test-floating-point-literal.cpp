// unit-test-floating-point-literal.cpp
//
// Regression coverage for #11276 — `getFloatingPointLiteralValue` must
// parse literals correctly regardless of the global C locale set by an
// embedding application. The previous implementation used `strtod`, which
// honours `LC_NUMERIC`; the current implementation uses
// `std::from_chars`, which is locale-independent by spec.

#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/compiler-core/slang-token.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

#include <clocale>
#include <cmath>

using namespace Slang;

namespace
{
double parseLiteral(const char* text)
{
    UnownedStringSlice content(text, text + ::strlen(text));
    Token token(TokenType::FloatingPointLiteral, content, SourceLoc());
    UnownedStringSlice suffix;
    return getFloatingPointLiteralValue(token, /*sink*/ nullptr, &suffix);
}

bool tryWithLocale(const char* localeName, void (*body)())
{
    const char* prev = std::setlocale(LC_NUMERIC, nullptr);
    String saved(prev ? prev : "C");

    if (!std::setlocale(LC_NUMERIC, localeName))
    {
        // Locale not installed on this host — skip the variant rather than
        // failing CI. We will at least have run the C-locale assertions.
        return false;
    }
    body();
    std::setlocale(LC_NUMERIC, saved.getBuffer());
    return true;
}
} // namespace

SLANG_UNIT_TEST(floatingPointLiteralValue)
{
    auto check = []()
    {
        // Plain decimal must parse with `.` as the radix character even if
        // the active LC_NUMERIC locale uses `,`.
        double pi = parseLiteral("3.14");
        SLANG_CHECK(pi > 3.139 && pi < 3.141);

        // Long-digit literal must round to the nearest `double`, not +INF
        // (the bug fixed in #11276 / #332).
        double longPi = parseLiteral(
            "3.141592653589793238462643383279502884197169399375105820974944"
            "5923078164062862089986280348253421170679821480865132823066");
        SLANG_CHECK(std::isfinite(longPi));
        SLANG_CHECK(longPi > 3.141 && longPi < 3.142);

        // C99 hex-float form is recognised.
        double tenViaHex = parseLiteral("0x1.4p+3");
        SLANG_CHECK(tenViaHex == 10.0);
    };

    // First run under whatever locale the test process was started with
    // (typically the C locale).
    check();

    // Then flip LC_NUMERIC to a comma-decimal locale and rerun. The
    // implementation must be unaffected. If the locale is not installed,
    // tryWithLocale skips this variant.
    tryWithLocale("de_DE.UTF-8", check);
    tryWithLocale("fr_FR.UTF-8", check);
}
