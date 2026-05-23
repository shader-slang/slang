// unit-test-repro-validator.cpp
// Tests for isReproStateValid and the ReproStateValidator graph traversal.

#include "core/slang-offset-container.h"
#include "slang/slang-repro-validator.h"
#include "slang/slang-repro.h"
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
        // "hello" is 5 chars; for short strings (length <= kSizeBase = 251)
        // the first byte encodes the length directly, so the header is 1 byte.
        // Layout: [5]['h']['e']['l']['l']['o']['\0']
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

    // 17. OffsetString with a one-byte extended size that exceeds the allocation — fails.
    // Exercises the multi-byte size header path in validateString.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
        auto strPtr = container.newString("hello");
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Corrupt: encode a one-byte extended size and claim a payload larger than
        // the "hello" string allocation.
        buf[strPtr.m_offset] = OffsetString::kSizeBase + 1;
        buf[strPtr.m_offset + 1] = 255;

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

    // 19. Long string (252+ chars) using multi-byte size encoding — passes.
    // Strings > kSizeBase (251) bytes use a multi-byte length header. This exercises the
    // sizeByteCount ∈ {1,2} success paths in validateString that the short-string tests skip.
    {
        // 252-byte string: first byte = kSizeBase+1 (252), 1 size byte = 252
        {
            OffsetContainer container;
            auto requestPtr = container.newObject<ReproUtil::RequestState>();
            auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);

            char longBuf252[253];
            memset(longBuf252, 'a', 252);
            longBuf252[252] = '\0';
            auto strPtr = container.newString(UnownedStringSlice(longBuf252, 252));
            container[requestPtr]->searchPaths = searchPathArray;
            container[searchPathArray[0]] = strPtr;

            List<uint8_t> buf;
            containerToBuffer(container, buf);
            SLANG_CHECK(isReproStateValid(buf));
        }

        // 260-byte string: first byte = kSizeBase+2 (253), 2 size bytes = 260
        {
            OffsetContainer container;
            auto requestPtr = container.newObject<ReproUtil::RequestState>();
            auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);

            char longBuf260[261];
            memset(longBuf260, 'b', 260);
            longBuf260[260] = '\0';
            auto strPtr = container.newString(UnownedStringSlice(longBuf260, 260));
            container[requestPtr]->searchPaths = searchPathArray;
            container[searchPathArray[0]] = strPtr;

            List<uint8_t> buf;
            containerToBuffer(container, buf);
            SLANG_CHECK(isReproStateValid(buf));
        }
    }

    // 19b. Short-form upper boundary: 251-byte string (firstByte == kSizeBase
    // uses the short branch with stringSize = firstByte). Locks in the
    // `firstByte <= kSizeBase` predicate against a future `<` regression.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);

        char longBuf251[252];
        memset(longBuf251, 'c', 251);
        longBuf251[251] = '\0';
        auto strPtr = container.newString(UnownedStringSlice(longBuf251, 251));
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19c. validatePathInfoMap: PathAndPathInfo with non-null path and null
    // pathInfo is allowed by the `if (pathInfo && ...)` short-circuit. Locks
    // that contract in so tightening it to require non-null pathInfo would
    // break this test.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);
        auto pathStr = container.newString("foo");

        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        container[pathAndPathInfoArray[0]].path = pathStr;
        // pathInfo is left null (zero-init) — must be accepted.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19d. validateStringPairArray positive case: RequestState.preprocessorDefinitions
    // with one StringPair holding non-null first and second strings passes.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto defines = container.newArray<ReproUtil::StringPair>(1);
        auto nameStr = container.newString("FOO");
        auto valueStr = container.newString("1");

        container[requestPtr]->preprocessorDefinitions = defines;
        container[defines[0]].first = nameStr;
        container[defines[0]].second = valueStr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19e. validateStringPairArray with null first — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto defines = container.newArray<ReproUtil::StringPair>(1);
        auto valueStr = container.newString("1");

        container[requestPtr]->preprocessorDefinitions = defines;
        // first left null (zero-init)
        container[defines[0]].second = valueStr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19f. validateStringPairArray with null second — fails.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto defines = container.newArray<ReproUtil::StringPair>(1);
        auto nameStr = container.newString("FOO");

        container[requestPtr]->preprocessorDefinitions = defines;
        container[defines[0]].first = nameStr;
        // second left null (zero-init)

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19g. validateOutputStateArray with valid entryPointIndex but corrupted
    // outputPath string — fails. Exercises the
    // `if (!validateString(outputState.outputPath))` branch.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        // entryPointCount = 1 so that entryPointIndex = 0 is in range.
        auto entryPoints = container.newArray<ReproUtil::EntryPointState>(1);
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);
        auto outputPathStr = container.newString("out.spv");

        container[requestPtr]->entryPoints = entryPoints;
        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[outputArray[0]].entryPointIndex = 0;
        container[outputArray[0]].outputPath = outputPathStr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        // First validate the buffer is otherwise well-formed.
        SLANG_CHECK(isReproStateValid(buf));

        // Corrupt: encode a one-byte extended size and claim a payload larger than
        // the outputPath string allocation.
        buf[outputPathStr.m_offset] = OffsetString::kSizeBase + 1;
        buf[outputPathStr.m_offset + 1] = 255;
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19h. validatePathInfoMap with null path — fails.
    // Exercises the `if (!validateRequiredString(paths[i].path))` branch.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);

        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        // path and pathInfo both left null (zero-init); validator requires path.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19i. validateStringPtrArray with requireElements = false: an entry point
    // whose specializationArgStrings is a 1-element array with a null element
    // must be accepted. Pins the only `requireElements = false` call site so a
    // future "harmonisation" to `true` would fail this test.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto tuArray = container.newArray<ReproUtil::TranslationUnitRequestState>(1);
        auto epArray = container.newArray<ReproUtil::EntryPointState>(1);
        auto specArgArray = container.newArray<Offset32Ptr<OffsetString>>(1);

        container[requestPtr]->translationUnits = tuArray;
        container[requestPtr]->entryPoints = epArray;
        // translationUnitIndex == 0 is valid (translationUnitCount == 1).
        container[epArray[0]].translationUnitIndex = 0;
        container[epArray[0]].specializationArgStrings = specArgArray;
        // specArgArray[0] is null by zero-init — must be accepted.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19j. Alignment check: an Offset32Array<T> whose m_data.m_offset is not
    // aligned to alignof(T) must be rejected. Pins the
    // `if (alignment > 1 && (offset % alignment) != 0)` branch in
    // isRangeInBounds against accidental removal.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        container[requestPtr]->files = filesArray;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // RequestState.files is an Offset32Array { m_data: Offset32Ptr (4
        // bytes), m_count: uint32_t (4 bytes) }. m_data.m_offset is the first
        // 4 bytes of the array struct, located at
        // kStartOffset + offsetof(RequestState, files).
        const size_t filesMDataOffset = kStartOffset + offsetof(ReproUtil::RequestState, files);
        uint32_t* mDataSlot = reinterpret_cast<uint32_t*>(buf.getBuffer() + filesMDataOffset);
        uint32_t originalOffset = *mDataSlot;
        SLANG_CHECK((originalOffset % 4) == 0);
        // +1 makes the array start misaligned for any T with alignment > 1.
        *mDataSlot = originalOffset + 1;
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19k. validateString with 4-byte multi-byte size encoding set to UINT32_MAX
    // is rejected — either via the overflow guard (on 32-bit size_t) or via the
    // subsequent buffer-bounds check (on 64-bit, where the claimed allocation
    // exceeds m_dataSize). Pins the upper-bound rejection regardless of host
    // word size.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto searchPathArray = container.newArray<Offset32Ptr<OffsetString>>(1);
        auto strPtr = container.newString("hello");
        container[requestPtr]->searchPaths = searchPathArray;
        container[searchPathArray[0]] = strPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);

        // Set firstByte to kSizeBase + 4 (4 size bytes follow) and the 4 size
        // bytes to 0xFF 0xFF 0xFF 0xFF, claiming a string of UINT32_MAX bytes.
        // The string allocation we have is far smaller, so the bounds check
        // (or overflow guard on 32-bit) must reject.
        buf[strPtr.m_offset] = OffsetString::kSizeBase + 4;
        // Ensure we have enough room to write the 4 size bytes.
        SLANG_CHECK(buf.getCount() >= strPtr.m_offset + 5);
        buf[strPtr.m_offset + 1] = 0xFF;
        buf[strPtr.m_offset + 2] = 0xFF;
        buf[strPtr.m_offset + 3] = 0xFF;
        buf[strPtr.m_offset + 4] = 0xFF;

        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19l. TranslationUnitRequestState.moduleName corruption — fails.
    // validateTranslationUnitArray calls validateString on moduleName; pin the
    // call so a future refactor that drops the TU-level check would fail this.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto tuArray = container.newArray<ReproUtil::TranslationUnitRequestState>(1);
        auto moduleNameStr = container.newString("my_module");

        container[requestPtr]->translationUnits = tuArray;
        container[tuArray[0]].moduleName = moduleNameStr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));

        // Corrupt: encode a one-byte extended size and claim a payload larger than
        // the moduleName string allocation.
        buf[moduleNameStr.m_offset] = OffsetString::kSizeBase + 1;
        buf[moduleNameStr.m_offset + 1] = 255;
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19m. OutputState.outputPath = null is silently accepted by
    // validateOutputStateArray (validateString returns true on null).
    // Mirrors the pathInfoMap null-pathInfo contract pinned by 19c.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        // entryPointCount = 1 so that entryPointIndex = 0 is in range.
        auto entryPoints = container.newArray<ReproUtil::EntryPointState>(1);
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);

        container[requestPtr]->entryPoints = entryPoints;
        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[outputArray[0]].entryPointIndex = 0;
        // outputPath left null (zero-init) — must be accepted.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 20. ReproUtil::getRequest size-guard boundary: N-1 / N / N+1, where
    // N = kStartOffset + sizeof(RequestState). Locks in the unconditional
    // minimum-size check getRequest applies before casting the payload.
    {
        const size_t N = kStartOffset + sizeof(ReproUtil::RequestState);

        List<uint8_t> tooSmall;
        tooSmall.setCount(N - 1);
        memset(tooSmall.getBuffer(), 0, tooSmall.getCount());
        SLANG_CHECK(ReproUtil::getRequest(tooSmall) == nullptr);

        List<uint8_t> exact;
        exact.setCount(N);
        memset(exact.getBuffer(), 0, exact.getCount());
        SLANG_CHECK(ReproUtil::getRequest(exact) != nullptr);

        List<uint8_t> oversize;
        oversize.setCount(N + 1);
        memset(oversize.getBuffer(), 0, oversize.getCount());
        SLANG_CHECK(ReproUtil::getRequest(oversize) != nullptr);
    }
}
