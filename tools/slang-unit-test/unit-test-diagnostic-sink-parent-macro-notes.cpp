// unit-test-diagnostic-sink-parent-macro-notes.cpp

#include "compiler-core/slang-core-diagnostics.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for the "expanded from macro" note dedup invariant documented in
// DiagnosticSink::diagnoseRichImpl (slang-diagnostic-sink.cpp): when a child sink forwards a
// diagnostic to a parent sink (DiagnosticSink::setParentSink), the child passes the *original*
// un-decorated diagnostic rather than its own already-noted copy, so the parent independently
// derives its own "expanded from macro 'X'" note via appendMacroExpansionNotes instead of
// receiving a second copy on top of the child's. Both the child's own rendered output and the
// parent's rendered output should therefore each contain exactly one occurrence of the note --
// neither zero (the note must still appear) nor two (it must not be duplicated).
SLANG_UNIT_TEST(diagnosticSinkParentSinkDoesNotDuplicateMacroExpansionNotes)
{
    SLANG_UNUSED(unitTestContext);

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    // A "definition file" standing in for the file containing the macro's body, and a
    // "call-site file" standing in for the file that invokes it. Real content doesn't matter
    // here -- only that each file has a SourceView backing a valid range of locs.
    SourceFile* defFile = sourceManager.createSourceFileWithString(
        PathInfo::makeUnknown(),
        "int BODY_TOKEN;\n");
    SourceView* defView = sourceManager.createSourceView(defFile, nullptr, SourceLoc());

    SourceFile* callSiteFile = sourceManager.createSourceFileWithString(
        PathInfo::makeUnknown(),
        "MACRO_CALL();\n");
    SourceView* callSiteView = sourceManager.createSourceView(callSiteFile, nullptr, SourceLoc());

    SourceLoc originalBodyBeginLoc = defView->getRange().begin;
    SourceLoc callSiteLoc = callSiteView->getRange().begin;

    // Register one macro invocation, mirroring what MacroInvocation's constructor does in
    // slang-preprocessor.cpp: this allocates a small per-invocation expansion range and remaps
    // the diagnostic's primary loc into it, so appendMacroExpansionNotes finds it via
    // SourceManager::findMacroExpansion and emits the "expanded from macro" note.
    SourceRange expansionRange = sourceManager.registerMacroExpansion(
        "MACRO_CALL",
        callSiteLoc,
        originalBodyBeginLoc,
        /* bodyRangeSize */ 1);
    SourceLoc diagnosticLoc = expansionRange.begin;

    DiagnosticSink parentSink(&sourceManager, nullptr);
    parentSink.setFlag(DiagnosticSink::Flag::AlwaysGenerateRichDiagnostics);

    DiagnosticSink childSink(&sourceManager, nullptr);
    childSink.setFlag(DiagnosticSink::Flag::AlwaysGenerateRichDiagnostics);
    childSink.setParentSink(&parentSink);

    // Any Error-severity DiagnosticInfo works here; the note-dedup behavior under test doesn't
    // depend on which diagnostic is being reported, only on where its primary loc is.
    childSink.diagnose(diagnosticLoc, MiscDiagnostics::unbalancedDownstreamArguments);

    auto countOccurrences = [](const String& haystack, const char* needle) -> Index
    {
        Index count = 0;
        Index searchStart = 0;
        for (;;)
        {
            Index found = haystack.indexOf(needle, searchStart);
            if (found < 0)
                break;
            count++;
            searchStart = found + 1;
        }
        return count;
    };

    const char* expandedFromMacroText = "expanded from macro";

    // The child sink's own rendered output must contain exactly one note.
    SLANG_CHECK(countOccurrences(childSink.outputBuffer, expandedFromMacroText) == 1);

    // The parent sink independently re-derives its own note set from the original diagnostic;
    // it must also see exactly one note -- not zero (lost) and not two (duplicated by receiving
    // a diagnostic that already carried the child's notes).
    SLANG_CHECK(countOccurrences(parentSink.outputBuffer, expandedFromMacroText) == 1);
}
