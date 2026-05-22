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
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
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
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
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
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
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
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
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
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
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
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);

        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[outputArray[0]].entryPointIndex = -1;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 10. EntryPointState translationUnitIndex equal to translationUnitCount (0 == 0) — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto entryPointArray = container.newArray<ReproUtil::EntryPointState>(1);

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
        auto entryPointArray = container.newArray<ReproUtil::EntryPointState>(1);

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
        auto srcFilesArray = container.newArray<Offset32Ptr<ReproUtil::SourceFileState>>(1);
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
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
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

    // 14. Array with m_count that would overflow when multiplied by element size — fails.
    // isArrayRangeInBounds: elementSize > size_t(-1) / count → return false.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        container[requestPtr]->files = filesArray;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Overwrite m_count to near-UINT32_MAX; elementSize * count would overflow size_t.
        uint32_t* countSlot =
            reinterpret_cast<uint32_t*>(buf.getBuffer() + filesArray.m_data.m_offset + 4);
        *countSlot = 0xFFFFFFFFu;

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 15. OffsetString with missing null terminator — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
        auto strPtr = container.newString("hello");
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Overwrite the null terminator with a non-zero byte.
        // "hello" is 5 chars; header is 1 byte (kSizeBase == 0 prefix for short strings,
        // so the first byte is the length itself). Layout: [5]['h']['e']['l']['l']['o']['\0']
        // The '\0' is at strPtr.m_offset + 1 + 5.
        buf[strPtr.m_offset + 1 + 5] = 'X';

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 16. SourceFileState with a valid file pointer whose contents is null — fails.
    // validateSourceFileState requires file->contents to be non-null.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto srcFilesArray = container.newArray<Offset32Ptr<ReproUtil::SourceFileState>>(1);
        auto srcFilePtr = container.newObject<ReproUtil::SourceFileState>();
        auto filePtr = container.newObject<ReproUtil::FileState>();
        // filePtr->contents is left null (zero-init).

        container[requestPtr]->sourceFiles = srcFilesArray;
        container[srcFilesArray[0]] = srcFilePtr;
        container[srcFilePtr]->file = filePtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 17. OffsetString with sizeByteCount == 5 (> 4) — fails.
    // validateString: sizeByteCount < 1 || sizeByteCount > 4 → return false.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
        auto strPtr = container.newString("hello");
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Corrupt: set first byte to kSizeBase + 5, claiming 5 size extension bytes.
        buf[strPtr.m_offset] = OffsetString::kSizeBase + 5;

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 18. FileState with both contents and uniqueName null (pathInfoMap crash path) — fails.
    // extractFiles() dereferences uniqueName unconditionally at slang-repro.cpp:1708 when
    // pathInfo->file is non-null, regardless of whether contents is set.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();

        // Build a pathInfoMap entry whose pathInfo->file references a FileState
        // with both contents and uniqueName null.
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);
        auto pathStr = container.newString("shader.slang");
        auto pathInfoPtr = container.newObject<ReproUtil::PathInfoState>();
        auto filePtr = container.newObject<ReproUtil::FileState>();
        // filePtr->contents and filePtr->uniqueName are both null (zero-init).

        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        container[pathAndPathInfoArray[0]].path = pathStr;
        container[pathAndPathInfoArray[0]].pathInfo = pathInfoPtr;
        container[pathInfoPtr]->file = filePtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }
}
