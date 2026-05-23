// unit-test-repro-validator.cpp
// Tests for isReproStateValid and the ReproStateValidator graph traversal.

#include "compiler-core/slang-diagnostic-sink.h"
#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "core/slang-offset-container.h"
#include "core/slang-stream.h"
#include "core/slang-string-util.h"
#include "slang.h"
#include "slang/slang-repro-validator.h"
#include "slang/slang-repro.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static const int kInvalidReproStateDiagnosticId = 99;

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

static uint32_t buildValidPathInfoMap(List<uint8_t>& outBuf)
{
    OffsetContainer container;
    auto requestPtr = container.newObject<ReproUtil::RequestState>();
    auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);
    auto pathInfoPtr = container.newObject<ReproUtil::PathInfoState>();

    container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
    container[pathAndPathInfoArray[0]].path = container.newString("shader.slang");
    container[pathAndPathInfoArray[0]].pathInfo = pathInfoPtr;

    containerToBuffer(container, outBuf);
    return pathInfoPtr.m_offset;
}

static uint32_t alignOffset(uint32_t offset, uint32_t alignment)
{
    const uint32_t remainder = offset % alignment;
    return remainder ? offset + alignment - remainder : offset;
}

static void buildSearchPathBufferWithStringSize(size_t stringSize, List<uint8_t>& outBuf)
{
    uint8_t encodedSize[OffsetString::kMaxSizeEncodeSize];
    const size_t headerSize = OffsetString::calcEncodedSize(stringSize, encodedSize);
    const uint32_t requestOffset = kStartOffset;
    const uint32_t arrayOffset = alignOffset(
        requestOffset + uint32_t(sizeof(ReproUtil::RequestState)),
        uint32_t(alignof(Offset32Ptr<OffsetString>)));
    const uint32_t stringOffset = alignOffset(
        arrayOffset + uint32_t(sizeof(Offset32Ptr<OffsetString>)),
        uint32_t(alignof(OffsetString)));

    outBuf.setCount(size_t(stringOffset) + headerSize + stringSize + 1);
    memset(outBuf.getBuffer(), 0, outBuf.getCount());

    auto request = reinterpret_cast<ReproUtil::RequestState*>(outBuf.getBuffer() + requestOffset);
    request->searchPaths.m_data.m_offset = arrayOffset;
    request->searchPaths.m_count = 1;

    auto searchPath =
        reinterpret_cast<Offset32Ptr<OffsetString>*>(outBuf.getBuffer() + arrayOffset);
    searchPath->m_offset = stringOffset;

    memcpy(outBuf.getBuffer() + stringOffset, encodedSize, headerSize);
}

static void corruptStringToOversizedPayload(
    List<uint8_t>& buffer,
    Offset32Ptr<OffsetString> stringPtr)
{
    SLANG_CHECK_ABORT(!stringPtr.isNull());
    SLANG_CHECK_ABORT(buffer.getCount() >= stringPtr.m_offset + 2);
    buffer[stringPtr.m_offset] = OffsetString::kSizeBase + 1;
    buffer[stringPtr.m_offset + 1] = 255;
}

static bool outputContainsDiagnosticId(DiagnosticSink& sink, Severity severity, int diagnosticId)
{
    String idString(diagnosticId);
    StringBuilder expected;
    expected << getSeverityName(severity) << "[E";
    expected.appendRepeatedChar('0', Math::Max<Index>(0, 5 - idString.getLength()));
    expected << idString << "]:";
    return sink.outputBuffer.produceString().contains(expected.produceString());
}

static void buildStateRiff(const List<uint8_t>& payload, List<uint8_t>& outRiff)
{
    RIFF::Builder riff;
    RIFF::BuildCursor cursor(riff);

    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, ReproUtil::kSlangStateFileFourCC);
        SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, ReproUtil::kSlangStateDataFourCC);

        ReproUtil::Header header;
        header.m_semanticVersion = SemanticVersion(
            ReproUtil::kMajorVersion,
            ReproUtil::kMinorVersion,
            ReproUtil::kPatchVersion);
        header.m_typeHash = ReproUtil::getTypeHash();

        cursor.addData(header);
        if (payload.getCount())
        {
            cursor.addUnownedData(payload.getBuffer(), payload.getCount());
        }
    }

    OwnedMemoryStream stream(FileAccess::Write);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(riff.writeTo(&stream)));
    stream.swapContents(outRiff);
}

static void loadReproBlobState(ISlangBlob* reproBlob, List<uint8_t>& outBuffer)
{
    SLANG_CHECK_ABORT(reproBlob && reproBlob->getBufferSize() != 0);

    DiagnosticSink sink;
    SlangResult loadResult = ReproUtil::loadState(
        static_cast<const uint8_t*>(reproBlob->getBufferPointer()),
        reproBlob->getBufferSize(),
        &sink,
        outBuffer);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadResult));
    SLANG_CHECK_ABORT(sink.getErrorCount() == 0);
    SLANG_CHECK_ABORT(isReproStateValid(outBuffer));
    SLANG_CHECK_ABORT(ReproUtil::getRequest(outBuffer) != nullptr);
}

static const char* getTranslationUnitSourceFilePath(
    MemoryOffsetBase& base,
    ReproUtil::RequestState* requestState,
    Index translationUnitIndex,
    Index sourceFileIndex)
{
    SLANG_CHECK_ABORT(requestState);
    SLANG_CHECK_ABORT(translationUnitIndex >= 0);
    SLANG_CHECK_ABORT(translationUnitIndex < requestState->translationUnits.getCount());

    const auto& translationUnit = base.asRaw(requestState->translationUnits[translationUnitIndex]);
    SLANG_CHECK_ABORT(sourceFileIndex >= 0);
    SLANG_CHECK_ABORT(sourceFileIndex < translationUnit.sourceFiles.getCount());

    Offset32Ptr<ReproUtil::SourceFileState> sourceFilePtr =
        base.asRaw(translationUnit.sourceFiles[sourceFileIndex]);
    auto sourceFile = base.asRaw(sourceFilePtr);
    SLANG_CHECK_ABORT(sourceFile);

    auto foundPath = base.asRaw(sourceFile->foundPath);
    SLANG_CHECK_ABORT(foundPath);
    return foundPath->getCstr();
}

static Offset32Ptr<ReproUtil::SourceFileState> addSourceFileState(
    OffsetContainer& container,
    const char* path)
{
    auto sourceFile = container.newObject<ReproUtil::SourceFileState>();
    auto file = container.newObject<ReproUtil::FileState>();
    auto pathString = container.newString(path);
    auto contents = container.newString("void test() {}");

    container[file]->contents = contents;
    container[file]->foundPath = pathString;
    container[file]->uniqueName = pathString;
    container[file]->uniqueIdentity = pathString;

    container[sourceFile]->foundPath = pathString;
    container[sourceFile]->file = file;

    return sourceFile;
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

    // 19a. Large strings using 3-byte and 4-byte size encodings pass.
    // The validator only reads the size header and null terminator, so these
    // buffers can be built directly without filling the whole payload.
    {
        List<uint8_t> buf;

        buildSearchPathBufferWithStringSize(size_t(1) << 16, buf);
        SLANG_CHECK(isReproStateValid(buf));

        buildSearchPathBufferWithStringSize(size_t(1) << 24, buf);
        SLANG_CHECK(isReproStateValid(buf));
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
        auto translationUnits = container.newArray<ReproUtil::TranslationUnitRequestState>(1);
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);
        auto outputPathStr = container.newString("out.spv");

        container[requestPtr]->entryPoints = entryPoints;
        container[requestPtr]->translationUnits = translationUnits;
        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[entryPoints[0]].translationUnitIndex = 0;
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

    // 19h2. validatePathInfoMap rejects duplicate path keys. load() inserts
    // pathInfoMap entries with Dictionary::add, so duplicate serialized keys
    // must be rejected before replay reaches the loader.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(2);

        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        container[pathAndPathInfoArray[0]].path = container.newString("shader.slang");
        container[pathAndPathInfoArray[1]].path = container.newString("shader.slang");

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19h3. Distinct FileState objects with the same non-empty uniqueIdentity
    // are rejected because load()/loadFileSystem populate unique maps with
    // Dictionary::add.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(2);
        auto file0 = container.newObject<ReproUtil::FileState>();
        auto file1 = container.newObject<ReproUtil::FileState>();

        container[requestPtr]->files = filesArray;
        container[filesArray[0]] = file0;
        container[filesArray[1]] = file1;
        container[file0]->uniqueName = container.newString("file0.slang");
        container[file0]->uniqueIdentity = container.newString("same-identity");
        container[file1]->uniqueName = container.newString("file1.slang");
        container[file1]->uniqueIdentity = container.newString("same-identity");

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19h4. Reusing the same FileState object is still accepted; the loader
    // interns FileState* to one PathInfo, so only distinct objects with duplicate
    // identities are malformed.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(2);
        auto file = container.newObject<ReproUtil::FileState>();

        container[requestPtr]->files = filesArray;
        container[filesArray[0]] = file;
        container[filesArray[1]] = file;
        container[file]->uniqueName = container.newString("file.slang");
        container[file]->uniqueIdentity = container.newString("same-object");

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19h5. The same FileState can be reached through requestState->files and
    // pathInfoMap.pathInfo.file. The validator must memoize by object address so
    // a shared file does not trip duplicate uniqueIdentity checks.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto filesArray = container.newArray<Offset32Ptr<ReproUtil::FileState>>(1);
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);
        auto pathInfoPtr = container.newObject<ReproUtil::PathInfoState>();
        auto file = container.newObject<ReproUtil::FileState>();

        container[requestPtr]->files = filesArray;
        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        container[filesArray[0]] = file;
        container[file]->uniqueName = container.newString("shared.slang");
        container[file]->uniqueIdentity = container.newString("shared-identity");
        container[pathAndPathInfoArray[0]].path = container.newString("shared.slang");
        container[pathAndPathInfoArray[0]].pathInfo = pathInfoPtr;
        container[pathInfoPtr]->file = file;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));

        const size_t fileOffset = pathInfoPtr.m_offset + offsetof(ReproUtil::PathInfoState, file);
        uint32_t* fileSlot = reinterpret_cast<uint32_t*>(buf.getBuffer() + fileOffset);
        *fileSlot = uint32_t(buf.getCount() + 100);
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
        auto translationUnits = container.newArray<ReproUtil::TranslationUnitRequestState>(1);
        auto targetArray = container.newArray<ReproUtil::TargetRequestState>(1);
        auto outputArray = container.newArray<ReproUtil::OutputState>(1);

        container[requestPtr]->entryPoints = entryPoints;
        container[requestPtr]->translationUnits = translationUnits;
        container[requestPtr]->targetRequests = targetArray;
        container[targetArray[0]].outputStates = outputArray;
        container[entryPoints[0]].translationUnitIndex = 0;
        container[outputArray[0]].entryPointIndex = 0;
        // outputPath left null (zero-init) — must be accepted.

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));
    }

    // 19n. FileState optional string fields are validated when present.
    // Corrupt each field in isolation so dropping one validateString term
    // fails a focused case.
    {
        typedef ReproUtil::FileState FileState;
        typedef Offset32Ptr<OffsetString> FileState::*FileStringMember;
        struct Case
        {
            FileStringMember member;
            const char* value;
        };
        const Case cases[] = {
            {&FileState::uniqueIdentity, "identity"},
            {&FileState::canonicalPath, "canonical.slang"},
            {&FileState::foundPath, "found.slang"},
        };

        for (const Case& testCase : cases)
        {
            OffsetContainer container;
            auto requestPtr = container.newObject<ReproUtil::RequestState>();
            auto filesArray = container.newArray<Offset32Ptr<FileState>>(1);
            auto filePtr = container.newObject<FileState>();

            container[requestPtr]->files = filesArray;
            container[filesArray[0]] = filePtr;
            container[filePtr]->uniqueIdentity = container.newString("identity");
            container[filePtr]->contents = container.newString("int main() {}");
            container[filePtr]->canonicalPath = container.newString("canonical.slang");
            container[filePtr]->foundPath = container.newString("found.slang");
            container[filePtr]->uniqueName = container.newString("unique.slang");
            container[filePtr]->*(testCase.member) = container.newString(testCase.value);

            List<uint8_t> buf;
            containerToBuffer(container, buf);
            SLANG_CHECK(isReproStateValid(buf));

            corruptStringToOversizedPayload(buf, container[filePtr]->*(testCase.member));
            SLANG_CHECK(!isReproStateValid(buf));
        }
    }

    // 19o. SourceFileState.foundPath is validated separately from the required file.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto sourceFilesArray = container.newArray<Offset32Ptr<ReproUtil::SourceFileState>>(1);
        auto sourceFilePtr = addSourceFileState(container, "source.slang");

        container[requestPtr]->sourceFiles = sourceFilesArray;
        container[sourceFilesArray[0]] = sourceFilePtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));

        corruptStringToOversizedPayload(buf, container[sourceFilePtr]->foundPath);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19p. PathInfoState.file rejects a non-null offset that points outside the buffer.
    {
        OffsetContainer container;
        auto requestPtr = container.newObject<ReproUtil::RequestState>();
        auto pathAndPathInfoArray = container.newArray<ReproUtil::PathAndPathInfo>(1);
        auto pathStr = container.newString("shader.slang");
        auto pathInfoPtr = container.newObject<ReproUtil::PathInfoState>();

        container[requestPtr]->pathInfoMap = pathAndPathInfoArray;
        container[pathAndPathInfoArray[0]].path = pathStr;
        container[pathAndPathInfoArray[0]].pathInfo = pathInfoPtr;

        List<uint8_t> buf;
        containerToBuffer(container, buf);
        SLANG_CHECK(isReproStateValid(buf));

        const size_t fileOffset = pathInfoPtr.m_offset + offsetof(ReproUtil::PathInfoState, file);
        uint32_t* fileSlot = reinterpret_cast<uint32_t*>(buf.getBuffer() + fileOffset);
        *fileSlot = uint32_t(buf.getCount() + 100);
        SLANG_CHECK(!isReproStateValid(buf));
    }

    // 19q. PathInfoState scalar fields reject invalid serialized enum values.
    {
        {
            List<uint8_t> buf;
            const uint32_t pathInfoOffset = buildValidPathInfoMap(buf);
            SLANG_CHECK(isReproStateValid(buf));

            auto pathType = reinterpret_cast<SlangPathType*>(
                buf.getBuffer() + pathInfoOffset + offsetof(ReproUtil::PathInfoState, pathType));
            *pathType = SlangPathType(2);
            SLANG_CHECK(!isReproStateValid(buf));
        }

        {
            List<uint8_t> buf;
            const uint32_t pathInfoOffset = buildValidPathInfoMap(buf);
            auto loadFileResult = reinterpret_cast<ReproUtil::PathInfoState::CompressedResult*>(
                buf.getBuffer() + pathInfoOffset +
                offsetof(ReproUtil::PathInfoState, loadFileResult));
            *loadFileResult = ReproUtil::PathInfoState::CompressedResult(0xFF);
            SLANG_CHECK(!isReproStateValid(buf));
        }

        {
            List<uint8_t> buf;
            const uint32_t pathInfoOffset = buildValidPathInfoMap(buf);
            auto getPathTypeResult = reinterpret_cast<ReproUtil::PathInfoState::CompressedResult*>(
                buf.getBuffer() + pathInfoOffset +
                offsetof(ReproUtil::PathInfoState, getPathTypeResult));
            *getPathTypeResult = ReproUtil::PathInfoState::CompressedResult(0xFF);
            SLANG_CHECK(!isReproStateValid(buf));
        }

        {
            List<uint8_t> buf;
            const uint32_t pathInfoOffset = buildValidPathInfoMap(buf);
            auto getCanonicalPathResult =
                reinterpret_cast<ReproUtil::PathInfoState::CompressedResult*>(
                    buf.getBuffer() + pathInfoOffset +
                    offsetof(ReproUtil::PathInfoState, getCanonicalPathResult));
            *getCanonicalPathResult = ReproUtil::PathInfoState::CompressedResult(0xFF);
            SLANG_CHECK(!isReproStateValid(buf));
        }
    }

    // 19r. OffsetBase pointer access returns null for an out-of-bounds offset.
    {
        List<uint8_t> buf;
        buildMinimalValid(buf);

        MemoryOffsetBase base;
        base.set(buf.getBuffer(), buf.getCount());
        Offset32Ptr<ReproUtil::RequestState> badPtr(uint32_t(buf.getCount() + 100));
        SLANG_CHECK(base[badPtr] == nullptr);
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

    // 21. Exported loader helpers fail cleanly on null request pointers.
    {
        List<uint8_t> buf;
        buildMinimalValid(buf);

        MemoryOffsetBase base;
        base.set(buf.getBuffer(), buf.getCount());

        SLANG_CHECK(SLANG_FAILED(ReproUtil::load(base, nullptr, nullptr, nullptr)));

        ComPtr<ISlangMutableFileSystem> mutableFileSystem(new MemoryFileSystem);
        SLANG_CHECK(SLANG_FAILED(ReproUtil::extractFiles(base, nullptr, mutableFileSystem)));
        SLANG_CHECK(
            SLANG_FAILED(ReproUtil::extractFiles(base, ReproUtil::getRequest(buf), nullptr)));
    }
}

SLANG_UNIT_TEST(reproStateLoadStateRejectsInvalidPayload)
{
    List<uint8_t> invalidPayload;
    buildMinimalValid(invalidPayload);
    SLANG_CHECK_ABORT(invalidPayload.getCount() > 0);
    invalidPayload.setCount(invalidPayload.getCount() - 1);

    List<uint8_t> riffData;
    buildStateRiff(invalidPayload, riffData);

    OwnedMemoryStream stream(FileAccess::Read);
    stream.setContent(riffData.getBuffer(), riffData.getCount());

    DiagnosticSink sink;
    List<uint8_t> outBuffer;
    outBuffer.add(0xff);

    SlangResult result = ReproUtil::loadState(&stream, &sink, outBuffer);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(outBuffer.getCount() == 0);
    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(outputContainsDiagnosticId(sink, Severity::Error, kInvalidReproStateDiagnosticId));
}

SLANG_UNIT_TEST(reproStateLoadStateRejectsEmptyPayload)
{
    List<uint8_t> emptyPayload;
    List<uint8_t> riffData;
    buildStateRiff(emptyPayload, riffData);

    OwnedMemoryStream stream(FileAccess::Read);
    stream.setContent(riffData.getBuffer(), riffData.getCount());

    DiagnosticSink sink;
    List<uint8_t> outBuffer;
    outBuffer.add(0xff);

    SlangResult result = ReproUtil::loadState(&stream, &sink, outBuffer);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(outBuffer.getCount() == 0);
    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(outputContainsDiagnosticId(sink, Severity::Error, kInvalidReproStateDiagnosticId));
}

SLANG_UNIT_TEST(reproStateValidatorAcceptsSavedState)
{
    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spEnableReproCapture(request)));
    spAddCodeGenTarget(request, SLANG_HLSL);

    int translationUnitIndex =
        spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "roundTrip");
    spAddTranslationUnitSourceString(
        request,
        translationUnitIndex,
        "round-trip.slang",
        "[shader(\"compute\")]\n"
        "[numthreads(1, 1, 1)]\n"
        "void computeMain() {}\n");
    spAddEntryPoint(request, translationUnitIndex, "computeMain", SLANG_STAGE_COMPUTE);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spCompile(request)));

    ComPtr<ISlangBlob> reproBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spSaveRepro(request, reproBlob.writeRef())));
    SLANG_CHECK_ABORT(reproBlob && reproBlob->getBufferSize() != 0);

    List<uint8_t> outBuffer;
    loadReproBlobState(reproBlob, outBuffer);

    auto replayRequest = spCreateCompileRequest(session);
    SLANG_CHECK_ABORT(replayRequest != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spLoadRepro(
        replayRequest,
        nullptr,
        reproBlob->getBufferPointer(),
        reproBlob->getBufferSize())));

    spDestroyCompileRequest(replayRequest);
    spDestroyCompileRequest(request);
    spDestroySession(session);
}

SLANG_UNIT_TEST(reproExtractFilesToDirectoryRejectsInvalidPayload)
{
    List<uint8_t> invalidPayload;
    buildMinimalValid(invalidPayload);
    SLANG_CHECK_ABORT(invalidPayload.getCount() > 0);
    invalidPayload.setCount(invalidPayload.getCount() - 1);

    List<uint8_t> riffData;
    buildStateRiff(invalidPayload, riffData);

    String filePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-repro"), filePath)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllBytes(filePath, riffData.getBuffer(), riffData.getCount())));

    String outputDir;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(ReproUtil::calcDirectoryPathFromFilename(filePath, outputDir)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Path::removeNonEmpty(outputDir)));

    DiagnosticSink sink;
    SlangResult result = ReproUtil::extractFilesToDirectory(filePath, &sink);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(outputContainsDiagnosticId(sink, Severity::Error, kInvalidReproStateDiagnosticId));
    SLANG_CHECK(!File::exists(outputDir));

    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(filePath)));
    SLANG_CHECK(SLANG_SUCCEEDED(Path::removeNonEmpty(outputDir)));
}

SLANG_UNIT_TEST(reproExtractFilesUsesSourceFileElementIndex)
{
    typedef ReproUtil::RequestState RequestState;
    typedef ReproUtil::SourceFileState SourceFileState;
    typedef ReproUtil::TranslationUnitRequestState TranslationUnitRequestState;

    OffsetContainer container;
    auto requestPtr = container.newObject<RequestState>();
    auto translationUnits = container.newArray<TranslationUnitRequestState>(2);
    auto tu0SourceFiles = container.newArray<Offset32Ptr<SourceFileState>>(1);
    auto tu1SourceFiles = container.newArray<Offset32Ptr<SourceFileState>>(2);
    auto tu0SourceFile = addSourceFileState(container, "tu0.slang");
    auto tu1SourceFileA = addSourceFileState(container, "tu1-a.slang");
    auto tu1SourceFileB = addSourceFileState(container, "tu1-b.slang");

    container[requestPtr]->translationUnits = translationUnits;
    container[translationUnits[0]].language = SourceLanguage::Slang;
    container[translationUnits[0]].sourceFiles = tu0SourceFiles;
    container[translationUnits[1]].language = SourceLanguage::Slang;
    container[translationUnits[1]].sourceFiles = tu1SourceFiles;

    container[tu0SourceFiles[0]] = tu0SourceFile;
    container[tu1SourceFiles[0]] = tu1SourceFileA;
    container[tu1SourceFiles[1]] = tu1SourceFileB;

    OffsetBase& base = container.asBase();
    ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(ReproUtil::extractFiles(base, base.asRaw(requestPtr), fileSystem)));

    ComPtr<ISlangBlob> manifestBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(fileSystem->loadFile("manifest.txt", manifestBlob.writeRef())));

    String manifest = StringUtil::getString(manifestBlob);
    Index tu0Index = manifest.indexOf(UnownedStringSlice("tu0.slang"));
    Index tu1AIndex = manifest.indexOf(UnownedStringSlice("tu1-a.slang"));
    Index tu1BIndex = manifest.indexOf(UnownedStringSlice("tu1-b.slang"));

    SLANG_CHECK(tu0Index >= 0);
    SLANG_CHECK(tu1AIndex >= 0);
    SLANG_CHECK(tu1BIndex >= 0);
    SLANG_CHECK(tu0Index < tu1AIndex);
    SLANG_CHECK(tu1AIndex < tu1BIndex);
}

SLANG_UNIT_TEST(reproLoadUsesSourceFileElementIndex)
{
    typedef ReproUtil::RequestState RequestState;
    typedef ReproUtil::SourceFileState SourceFileState;
    typedef ReproUtil::TranslationUnitRequestState TranslationUnitRequestState;

    OffsetContainer container;
    auto requestPtr = container.newObject<RequestState>();
    auto translationUnits = container.newArray<TranslationUnitRequestState>(2);
    auto tu0SourceFiles = container.newArray<Offset32Ptr<SourceFileState>>(1);
    auto tu1SourceFiles = container.newArray<Offset32Ptr<SourceFileState>>(2);
    auto tu0SourceFile = addSourceFileState(container, "load-tu0.slang");
    auto tu1SourceFileA = addSourceFileState(container, "load-tu1-a.slang");
    auto tu1SourceFileB = addSourceFileState(container, "load-tu1-b.slang");

    container[requestPtr]->translationUnits = translationUnits;
    container[translationUnits[0]].language = SourceLanguage::Slang;
    container[translationUnits[0]].sourceFiles = tu0SourceFiles;
    container[translationUnits[1]].language = SourceLanguage::Slang;
    container[translationUnits[1]].sourceFiles = tu1SourceFiles;

    container[tu0SourceFiles[0]] = tu0SourceFile;
    container[tu1SourceFiles[0]] = tu1SourceFileA;
    container[tu1SourceFiles[1]] = tu1SourceFileB;

    List<uint8_t> payload;
    containerToBuffer(container, payload);
    List<uint8_t> riffData;
    buildStateRiff(payload, riffData);

    auto session = spCreateSession();
    auto externalRequest = spCreateCompileRequest(session);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        spLoadRepro(externalRequest, nullptr, riffData.getBuffer(), riffData.getCount())));
    SLANG_CHECK_ABORT(spGetTranslationUnitCount(externalRequest) == 2);

    ComPtr<ISlangBlob> savedReproBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spSaveRepro(externalRequest, savedReproBlob.writeRef())));

    List<uint8_t> loadedBuffer;
    loadReproBlobState(savedReproBlob, loadedBuffer);

    MemoryOffsetBase loadedBase;
    loadedBase.set(loadedBuffer.getBuffer(), loadedBuffer.getCount());
    auto loadedRequestState = ReproUtil::getRequest(loadedBuffer);
    SLANG_CHECK_ABORT(loadedRequestState->translationUnits.getCount() == 2);
    const auto& loadedTu0 = loadedBase.asRaw(loadedRequestState->translationUnits[0]);
    const auto& loadedTu1 = loadedBase.asRaw(loadedRequestState->translationUnits[1]);

    SLANG_CHECK_ABORT(loadedTu0.sourceFiles.getCount() == 1);
    SLANG_CHECK_ABORT(loadedTu1.sourceFiles.getCount() == 2);
    SLANG_CHECK(
        strcmp(
            getTranslationUnitSourceFilePath(loadedBase, loadedRequestState, 0, 0),
            "load-tu0.slang") == 0);
    SLANG_CHECK(
        strcmp(
            getTranslationUnitSourceFilePath(loadedBase, loadedRequestState, 1, 0),
            "load-tu1-a.slang") == 0);
    SLANG_CHECK(
        strcmp(
            getTranslationUnitSourceFilePath(loadedBase, loadedRequestState, 1, 1),
            "load-tu1-b.slang") == 0);

    spDestroyCompileRequest(externalRequest);
    spDestroySession(session);
}
