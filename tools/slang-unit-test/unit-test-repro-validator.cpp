// unit-test-repro-validator.cpp
// Tests for isReproStateValid and the ReproStateValidator graph traversal.

#include "../../source/core/slang-offset-container.h"
#include "../../source/slang/slang-repro-validator.h"
#include "../../source/slang/slang-repro.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Copy OffsetContainer bytes into a List<uint8_t> as isReproStateValid expects.
static void containerToBuffer(OffsetContainer& container, List<uint8_t>& outBuf)
{
    size_t n = container.getDataCount();
    outBuf.setCount(n);
    memcpy(outBuf.getBuffer(), container.getData(), n);
}

// Build a minimal all-empty RequestState (no files, no source files, etc.).
// All arrays have count 0 and all optional pointers are null — this should pass.
static void buildMinimalValid(List<uint8_t>& outBuf)
{
    OffsetContainer container;
    container.newObject<ReproUtil::RequestState>();
    containerToBuffer(container, outBuf);
}

SLANG_UNIT_TEST(reproStateValidator)
{
    // 1. Minimal valid RequestState (all empty) passes.
    {
        List<uint8_t> buf;
        buildMinimalValid(buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 2. Empty buffer — fails (can't fit RequestState at kStartOffset).
    {
        List<uint8_t> buf;
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 3. Buffer smaller than kStartOffset + sizeof(RequestState) — fails.
    {
        List<uint8_t> buf;
        buf.setCount(4);
        memset(buf.getBuffer(), 0, 4);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 4. Offset32Ptr<FileState> pointing past the end of the buffer — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        // Allocate a one-element files array; element starts null (zero-init).
        auto filesArray =
            container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        container[requestPtr]->files = filesArray;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Corrupt: overwrite the element offset to point 100 bytes past the buffer.
        auto* fileOffsetSlot =
            reinterpret_cast<uint32_t*>(buf.getBuffer() + filesArray.m_data.m_offset);
        *fileOffsetSlot = uint32_t(buf.getCount() + 100);

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 5. searchPaths array with a null element — fails (requireElements == true).
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray =
            container.newArray<Offset32Ptr<OffsetString>>(1);
        container[requestPtr]->searchPaths = searchPathArray;
        // searchPathArray[0] is null by zero-init; requireElements rejects nulls.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 6. OffsetString with a size-extension header claiming more bytes than fit — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray =
            container.newArray<Offset32Ptr<OffsetString>>(1);
        auto strPtr = container.newString("hello");
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Corrupt: set the first byte of the string to kSizeBase + 4, which claims
        // 4 following size bytes, but the string is too short to hold them.
        buf[strPtr.m_offset] = OffsetString::kSizeBase + 4;

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 7. FileState with non-null contents but null uniqueName — fails.
    // extractFiles() dereferences uniqueName whenever contents is present.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray =
            container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        auto filePtr = container.newObject<ReproUtil::FileState>();
        auto contentsStr = container.newString("int main() {}");

        container[requestPtr]->files = filesArray;
        container[filesArray[0]] = filePtr;
        container[filePtr]->contents = contentsStr;
        // uniqueName left null (zero-init).

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 8. OutputState with entryPointIndex equal to entryPointCount (0 == 0) — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto targetArray =
            container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);

        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        // entryPointCount == 0; entryPointIndex == 0 is out of range.
        container[outputArray[0]].entryPointIndex = 0;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 9. OutputState with entryPointIndex == -1 — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto targetArray =
            container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);

        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[outputArray[0]].entryPointIndex = -1;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 10. EntryPointState with translationUnitIndex equal to translationUnitCount (0 == 0) — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto entryPointArray =
            container.newArray<ReproUtil::EntryPointState>(1);

        container[requestPtr]->entryPoints = entryPointArray;
        // translationUnitCount == 0; translationUnitIndex == 0 is out of range.
        container[entryPointArray[0]].translationUnitIndex = 0;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 11. EntryPointState with translationUnitIndex == UINT32_MAX — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto entryPointArray =
            container.newArray<ReproUtil::EntryPointState>(1);

        container[requestPtr]->entryPoints = entryPointArray;
        container[entryPointArray[0]].translationUnitIndex = UINT32_MAX;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 12. SourceFileState with a null file pointer — fails (requireFile == true).
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto srcFilesArray =
            container.newArray<Offset32Ptr<ReproUtil::SourceFileState>>(1);
        auto srcFilePtr = container.newObject<ReproUtil::SourceFileState>();

        container[requestPtr]->sourceFiles = srcFilesArray;
        container[srcFilesArray[0]] = srcFilePtr;
        // sourceFile->file is left null — requires a file with non-null contents.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 13. Valid buffer with one complete FileState (contents + uniqueName) passes.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray =
            container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        auto filePtr = container.newObject<ReproUtil::FileState>();
        auto contentsStr = container.newString("int main() {}");
        auto uniqueNameStr = container.newString("shader.slang");

        container[requestPtr]->files = filesArray;
        container[filesArray[0]] = filePtr;
        container[filePtr]->contents = contentsStr;
        container[filePtr]->uniqueName = uniqueNameStr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }
}
