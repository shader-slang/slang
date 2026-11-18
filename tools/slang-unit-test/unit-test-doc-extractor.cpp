// Documentation extraction owns a temporary lexical sequence for each file it visits.
// We check comment association and copied output text across a file change, including a
// trailing comment whose search crosses a declaration's semicolon.

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-doc-extractor.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Checks comment association across files and the lifetime of copied documentation text.
SLANG_UNIT_TEST(docExtractorAssociatesCommentsAcrossFiles)
{
    // We use different comment styles and declaration positions so stale lexemes from the
    // first file cannot accidentally produce the expected documentation for the second.
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, nullptr);
    auto firstFile = sourceManager.createSourceFileWithString(
        PathInfo::makeFromString("first.slang"),
        UnownedStringSlice("// First file.\n/// First.\nint first;\nint tail; ///< Tail.\n"));
    auto firstView = sourceManager.createSourceView(firstFile, nullptr, SourceLoc());
    auto secondFile = sourceManager.createSourceFileWithString(
        PathInfo::makeFromString("second.slang"),
        UnownedStringSlice(
            "// Second file.\n    /**Second.\n    Continued.\n    */\nint second;\n"));
    auto secondView = sourceManager.createSourceView(secondFile, nullptr, SourceLoc());

    // We search at the declaration names, using their spellings to locate them in the fixtures.
    auto firstNameOffset = firstFile->getContent().indexOf(UnownedStringSlice("first;"));
    auto tailNameOffset = firstFile->getContent().indexOf(UnownedStringSlice("tail;"));
    auto secondNameOffset = secondFile->getContent().indexOf(UnownedStringSlice("second;"));
    SLANG_CHECK_ABORT(firstNameOffset >= 0);
    SLANG_CHECK_ABORT(tailNameOffset >= 0);
    SLANG_CHECK_ABORT(secondNameOffset >= 0);

    // Inputs deliberately differ from file order. Each output identifies its original input,
    // and its view identifies the file whose comments were searched.
    using SearchStyle = DocMarkupExtractor::SearchStyle;
    DocMarkupExtractor::SearchItemInput inputs[] = {
        {secondView->getRange().getSourceLocFromOffset(uint32_t(secondNameOffset)),
         SearchStyle::Variable},
        {firstView->getRange().getSourceLocFromOffset(uint32_t(tailNameOffset)),
         SearchStyle::Variable},
        {firstView->getRange().getSourceLocFromOffset(uint32_t(firstNameOffset)),
         SearchStyle::Variable},
    };
    List<SourceView*> views;
    List<DocMarkupExtractor::SearchItemOutput> output;
    DocMarkupExtractor extractor;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        extractor.extract(inputs, SLANG_COUNT_OF(inputs), &sourceManager, &sink, views, output)));
    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK_ABORT(views.getCount() == 2);
    SLANG_CHECK(views[0] == firstView);
    SLANG_CHECK(views[1] == secondView);
    SLANG_CHECK_ABORT(output.getCount() == 3);

    // Extraction has visited both files and released its temporary token pools. The copied
    // strings must still describe the correct declarations, including the first file's text.
    SLANG_CHECK(output[0].inputIndex == 2);
    SLANG_CHECK(output[0].viewIndex == 0);
    SLANG_CHECK(output[0].text == "First.\n");
    SLANG_CHECK(output[1].inputIndex == 1);
    SLANG_CHECK(output[1].viewIndex == 0);
    SLANG_CHECK(output[1].text == "Tail.\n");
    SLANG_CHECK(output[2].inputIndex == 0);
    SLANG_CHECK(output[2].viewIndex == 1);
    SLANG_CHECK(output[2].text == "Second.\nContinued.\n");
}
