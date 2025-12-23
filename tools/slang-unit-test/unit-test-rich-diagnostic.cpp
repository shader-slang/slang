// unit-test-rich-diagnostic.cpp
// Unit tests for the rich diagnostic formatting system

#include "source/compiler-core/slang-diagnostic-sink.h"
#include "source/compiler-core/slang-rich-diagnostic-layout.h"
#include "source/compiler-core/slang-rich-diagnostic.h"
#include "source/compiler-core/slang-source-loc.h"
#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(richDiagnosticBasic)
{
    // Create a simple rich diagnostic and verify it renders correctly
    RichDiagnostic diagnostic;
    diagnostic.code = "E30019";
    diagnostic.severity = Severity::Error;
    diagnostic.message = "cannot convert argument of type `float` to parameter of type `int`";
    diagnostic.addNote("no implicit conversion exists from `float` to `int`");
    diagnostic.addHelp("add explicit cast: `(int)value`");

    // Verify the diagnostic was constructed correctly
    SLANG_CHECK(diagnostic.code == "E30019");
    SLANG_CHECK(diagnostic.severity == Severity::Error);
    SLANG_CHECK(diagnostic.notes.getCount() == 1);
    SLANG_CHECK(diagnostic.helps.getCount() == 1);
}

SLANG_UNIT_TEST(richDiagnosticBuilder)
{
    // Test the builder pattern
    RichDiagnostic diagnostic = RichDiagnosticBuilder()
                                    .setCode("E30019")
                                    .setSeverity(Severity::Error)
                                    .setMessage("type mismatch")
                                    .addNote("types must match")
                                    .addHelp("use explicit cast")
                                    .build();

    SLANG_CHECK(diagnostic.code == "E30019");
    SLANG_CHECK(diagnostic.severity == Severity::Error);
    SLANG_CHECK(diagnostic.message == "type mismatch");
    SLANG_CHECK(diagnostic.notes.getCount() == 1);
    SLANG_CHECK(diagnostic.helps.getCount() == 1);
}

SLANG_UNIT_TEST(richDiagnosticLayoutBasic)
{
    // Test the layout engine without source locations
    RichDiagnostic diagnostic;
    diagnostic.code = "E30019";
    diagnostic.severity = Severity::Error;
    diagnostic.message = "cannot convert argument of type `float` to parameter of type `int`";
    diagnostic.addNote("no implicit conversion exists from `float` to `int`");
    diagnostic.addHelp("add explicit cast: `(int)value`");

    // Create layout engine without source manager (for basic test)
    RichDiagnosticLayout layout(nullptr);
    String rendered = layout.render(diagnostic);

    // Verify the output contains expected elements
    SLANG_CHECK(rendered.indexOf("error[E30019]") >= 0);
    SLANG_CHECK(rendered.indexOf("cannot convert argument") >= 0);
    SLANG_CHECK(rendered.indexOf("= note:") >= 0);
    SLANG_CHECK(rendered.indexOf("= help:") >= 0);
}