// unit-test-macro-expansion-source-view-growth.cpp

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-name.h"
#include "compiler-core/slang-source-loc.h"
#include "slang/slang-preprocessor.h"
#include "slang/slang-profile.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for the perf bug that got PR #6165 reverted in #11116: the original
// "expanded from macro" implementation created a full SourceFile/SourceView per macro
// invocation (via createSourceFileWithBlob + createSourceView), so a translation unit with
// many invocations of a macro grew SourceManager::m_sourceViews linearly, slowing every
// subsequent SourceManager::findSourceView binary search.
//
// The side-table design this PR re-lands the feature with (registerMacroExpansion /
// findMacroExpansion in slang-source-loc.cpp) tracks each invocation as a small entry in a
// flat List<MacroExpansionEntry>, with no SourceFile or SourceView created per invocation.
// This test preprocesses a source file that invokes a body-having macro many times, and
// asserts that SourceManager::getSourceViews() does not grow at all as a result -- the only
// SourceView created for the whole run should be the one for the source file itself.
SLANG_UNIT_TEST(macroExpansionDoesNotGrowSourceViews)
{
    SLANG_UNUSED(unitTestContext);

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    DiagnosticSink sink(&sourceManager, nullptr);

    NamePool namePool;

    // A macro with a non-empty body, invoked many times. Each invocation used to allocate its
    // own SourceFile/SourceView; with the side-table design it should allocate none.
    StringBuilder source;
    source << "#define BODY(x) (x + 1)\n";
    source << "int main() {\n";
    source << "    int total = 0;\n";
    const int kInvocationCount = 200;
    for (int i = 0; i < kInvocationCount; ++i)
    {
        source << "    total = BODY(total);\n";
    }
    source << "    return total;\n";
    source << "}\n";

    SourceFile* sourceFile =
        sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), source.produceString());

    // createSourceFileWithString does not itself create a SourceView; that only happens when
    // the preprocessor (or some other consumer) calls createSourceView on the file. Capture the
    // count here so the assertion below measures growth from preprocessing alone.
    const Index viewCountBeforePreprocessing = sourceManager.getSourceViews().getCount();

    PreprocessorDesc desc;
    desc.sink = &sink;
    desc.namePool = &namePool;
    desc.fileSystem = nullptr; // No #include directives are used by this test's source.
    desc.sourceManager = &sourceManager;

    SourceLanguage detectedLanguage = SourceLanguage::Unknown;
    SlangLanguageVersion languageVersion = SLANG_LANGUAGE_VERSION_LATEST;
    preprocessSource(sourceFile, desc, detectedLanguage, languageVersion);

    // Preprocessing the file itself creates exactly one SourceView (for sourceFile). None of
    // the kInvocationCount macro invocations should create any additional SourceView: they are
    // tracked entirely through SourceManager's macro-expansion side table instead.
    const Index viewCountAfterPreprocessing = sourceManager.getSourceViews().getCount();
    SLANG_CHECK(viewCountAfterPreprocessing == viewCountBeforePreprocessing + 1);
}
