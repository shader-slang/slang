// unit-test-source-file-set-contents.cpp

#include "compiler-core/slang-source-loc.h"
#include "core/slang-crypto.h"
#include "core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static SHA1::Digest _computeDigest(const String& content)
{
    return SHA1::compute(content.getBuffer(), size_t(content.getLength()));
}

// SourceFile caches the digest and the line break offsets of its content. Both must be derived
// from the contents that were set last, including when they were queried before any contents
// were set (a SourceFile can be created with only a size and receive its contents later).
SLANG_UNIT_TEST(sourceFileSetContentsResetsCaches)
{
    SLANG_UNUSED(unitTestContext);

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    // The digest queried before the contents are set must not outlive setContents().
    {
        const String content = "abc\ndef";
        SourceFile* sourceFile =
            sourceManager.createSourceFileWithSize(PathInfo::makeUnknown(), content.getLength());
        SHA1::Digest emptyDigest = sourceFile->getDigest();
        SLANG_CHECK(emptyDigest != _computeDigest(content));

        sourceFile->setContents(content);
        SLANG_CHECK(sourceFile->getDigest() == _computeDigest(content));
    }

    // The digest and the line break offsets must follow replaced contents. setContents() asserts
    // that the new blob has the size the file currently reports, so the new contents have the same
    // size as the old ones (UTF-8 without a BOM keeps the raw size).
    {
        const String first = "ab\ncd\ne";
        const String second = "abcde\nf";
        SLANG_CHECK(first.getLength() == second.getLength());

        SourceFile* sourceFile =
            sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), first);
        SLANG_CHECK(sourceFile->getDigest() == _computeDigest(first));
        SLANG_CHECK(sourceFile->getLineBreakOffsets().getCount() == 3);
        SLANG_CHECK(sourceFile->calcLineIndexFromOffset(4) == 1);

        sourceFile->setContents(second);
        SLANG_CHECK(sourceFile->getDigest() == _computeDigest(second));
        const auto& offsets = sourceFile->getLineBreakOffsets();
        SLANG_CHECK(offsets.getCount() == 2);
        if (offsets.getCount() == 2)
        {
            SLANG_CHECK(offsets[0] == 0);
            SLANG_CHECK(offsets[1] == 6);
        }
        SLANG_CHECK(sourceFile->calcLineIndexFromOffset(4) == 0);
        SLANG_CHECK(sourceFile->calcLineIndexFromOffset(6) == 1);
    }
}
