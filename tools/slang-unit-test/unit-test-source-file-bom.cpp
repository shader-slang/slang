
#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/core/slang-blob.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for https://github.com/shader-slang/slang/issues/10616
//
// When a source file has a BOM or non-UTF-8 encoding, setContents() decodes/strips the
// BOM, producing a decoded buffer smaller than the raw file. Previously m_contentSize was
// not updated to the decoded size, causing getOffsetRangeAtLineIndex() to return
// out-of-bounds ranges for the last line, which led to crashes in calcCodePointCount().

static void _checkSourceFileBom(
    SourceManager& sourceManager,
    const void* rawData,
    size_t rawSize,
    size_t bomSize)
{
    auto blob = RawBlob::create(rawData, rawSize);
    SourceFile* sf = sourceManager.createSourceFileWithBlob(PathInfo::makeUnknown(), blob);

    // After setContents, getContentSize() must equal the actual decoded content length.
    SLANG_CHECK(sf->getContentSize() == size_t(sf->getContent().getLength()));

    // The decoded content must be smaller than the raw input by at least the BOM size.
    SLANG_CHECK(sf->getContentSize() < rawSize);
    SLANG_CHECK(rawSize - sf->getContentSize() >= bomSize);

    // Exercise the crash path: calcColumnIndex on the last line calls
    // getLineAtIndex -> getOffsetRangeAtLineIndex (uses getContentSize())
    // -> head(colOffset) -> calcCodePointCount. With the stale m_contentSize
    // this would read out of bounds.
    int lastCharOffset = int(sf->getContent().getLength()) - 2;
    int lineIndex = sf->calcLineIndexFromOffset(lastCharOffset);
    int col = sf->calcColumnIndex(lineIndex, lastCharOffset);
    SLANG_CHECK(col >= 0);
}

SLANG_UNIT_TEST(sourceFileBomContentSize)
{
    SLANG_UNUSED(unitTestContext);

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    // Test 1: UTF-8 BOM (3 bytes: EF BB BF)
    {
        const char raw[] = "\xEF\xBB\xBF"
                           "int x = 42;\nfloat y = 1.0;\n";
        _checkSourceFileBom(sourceManager, raw, sizeof(raw) - 1, 3);
    }

    // Test 2: UTF-16 LE BOM (2 bytes: FF FE) followed by UTF-16 LE encoded text.
    // "int a;\n" in UTF-16 LE is each ASCII char as (char, 0x00) pairs.
    {
        const Byte raw[] = {
            0xFF,
            0xFE, // UTF-16 LE BOM
            'i',
            0x00,
            'n',
            0x00,
            't',
            0x00, // "int"
            ' ',
            0x00,
            'a',
            0x00,
            ';',
            0x00, // " a;"
            '\n',
            0x00, // newline
        };
        _checkSourceFileBom(sourceManager, raw, sizeof(raw), 2);
    }
}
