
#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/core/slang-blob.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for https://github.com/shader-slang/slang/issues/10616
//
// When a source file has a UTF-8 BOM, setContents() decodes/strips the BOM, producing
// a decoded buffer smaller than the raw file. Previously m_contentSize was not updated
// to the decoded size, causing getOffsetRangeAtLineIndex() to return out-of-bounds ranges
// for the last line, which led to crashes in calcCodePointCount().

SLANG_UNIT_TEST(sourceFileBomContentSize)
{
    SLANG_UNUSED(unitTestContext);

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    // UTF-8 BOM (3 bytes: EF BB BF) followed by multi-line source text.
    const char raw[] = "\xEF\xBB\xBF"
                       "int x = 42;\nfloat y = 1.0;\n";
    const size_t rawSize = sizeof(raw) - 1; // exclude null terminator

    auto blob = RawBlob::create(raw, rawSize);
    SourceFile* sf = sourceManager.createSourceFileWithBlob(PathInfo::makeUnknown(), blob);

    // After setContents, getContentSize() must equal the actual decoded content length.
    // Before the fix, getContentSize() returned the raw size (with BOM), while
    // getContent() returned the decoded content (without BOM).
    SLANG_CHECK(sf->getContentSize() == size_t(sf->getContent().getLength()));

    // The decoded content should not contain the BOM bytes.
    SLANG_CHECK(sf->getContent().getLength() == rawSize - 3);

    // Exercise the crash path: calcColumnIndex on the last line calls
    // getLineAtIndex -> getOffsetRangeAtLineIndex (uses getContentSize())
    // -> head(colOffset) -> calcCodePointCount. With the stale m_contentSize
    // this would read out of bounds.
    SourceView* sv = sourceManager.createSourceView(sf, nullptr, SourceLoc());
    SLANG_UNUSED(sv);

    int lineIndex = sf->calcLineIndexFromOffset(int(sf->getContent().getLength()) - 2);
    int col = sf->calcColumnIndex(lineIndex, int(sf->getContent().getLength()) - 2);
    SLANG_CHECK(col >= 0);
}
