// slang-repro-validator.cpp
#include "slang-repro-validator.h"

#include "slang-repro.h"

namespace Slang
{
namespace
{

struct ReproStateValidator
{
    typedef ReproUtil::EntryPointState EntryPointState;
    typedef ReproUtil::FileState FileState;
    typedef ReproUtil::OutputState OutputState;
    typedef ReproUtil::PathAndPathInfo PathAndPathInfo;
    typedef ReproUtil::PathInfoState PathInfoState;
    typedef ReproUtil::RequestState RequestState;
    typedef ReproUtil::SourceFileState SourceFileState;
    typedef ReproUtil::StringPair StringPair;
    typedef ReproUtil::TargetRequestState TargetRequestState;
    typedef ReproUtil::TranslationUnitRequestState TranslationUnitRequestState;

    ReproStateValidator(const uint8_t* data, size_t dataSize)
        : m_data(data), m_dataSize(dataSize)
    {
    }

    bool validate() const
    {
        const RequestState* requestState = nullptr;
        return tryGetObjectAt(kStartOffset, requestState) && validateRequestState(*requestState);
    }

private:
    bool isRangeInBounds(uint32_t offset, size_t size, size_t alignment) const
    {
        if (offset == kNull32Offset)
        {
            return false;
        }

        if (!m_data)
        {
            return false;
        }

        if (alignment > 1 && (offset % alignment) != 0)
        {
            return false;
        }

        if (offset > m_dataSize)
        {
            return false;
        }

        return size <= m_dataSize - offset;
    }

    bool isArrayRangeInBounds(uint32_t offset, size_t elementSize, size_t count, size_t alignment)
        const
    {
        if (count == 0)
        {
            return true;
        }

        if (elementSize > size_t(-1) / count)
        {
            return false;
        }

        return isRangeInBounds(offset, elementSize * count, alignment);
    }

    template<typename T>
    bool tryGetObjectAt(uint32_t offset, const T*& outObject) const
    {
        outObject = nullptr;
        if (!isRangeInBounds(offset, sizeof(T), alignof(T)))
        {
            return false;
        }

        outObject = (const T*)(m_data + offset);
        return true;
    }

    template<typename T>
    bool tryGetObject(Offset32Ptr<T> ptr, const T*& outObject) const
    {
        outObject = nullptr;
        if (ptr.isNull())
        {
            return true;
        }

        return tryGetObjectAt(ptr.m_offset, outObject);
    }

    template<typename T>
    bool tryGetRequiredObject(Offset32Ptr<T> ptr, const T*& outObject) const
    {
        outObject = nullptr;
        return !ptr.isNull() && tryGetObjectAt(ptr.m_offset, outObject);
    }

    template<typename T>
    bool tryGetArrayData(const Offset32Array<T>& array, const T*& outData) const
    {
        outData = nullptr;
        if (array.m_count == 0)
        {
            return true;
        }

        if (!isArrayRangeInBounds(array.m_data.m_offset, sizeof(T), array.m_count, alignof(T)))
        {
            return false;
        }

        outData = (const T*)(m_data + array.m_data.m_offset);
        return true;
    }

    bool validateString(Offset32Ptr<OffsetString> ptr) const
    {
        if (ptr.isNull())
        {
            return true;
        }

        const uint32_t offset = ptr.m_offset;
        if (!isRangeInBounds(offset, 1, alignof(OffsetString)))
        {
            return false;
        }

        const uint8_t firstByte = m_data[offset];

        size_t headerSize = 1;
        size_t stringSize = 0;
        if (firstByte <= OffsetString::kSizeBase)
        {
            stringSize = firstByte;
        }
        else
        {
            static_assert(
                OffsetString::kSizeBase + 4 == 0xff,
                "OffsetString uses at most 4 size bytes");
            const int sizeByteCount = firstByte - OffsetString::kSizeBase;
            SLANG_ASSERT(sizeByteCount >= 1 && sizeByteCount <= 4);

            headerSize += size_t(sizeByteCount);
            if (!isRangeInBounds(offset, headerSize, alignof(OffsetString)))
            {
                return false;
            }

            for (int i = 0; i < sizeByteCount; ++i)
            {
                stringSize |= size_t(m_data[offset + 1 + i]) << (i * 8);
            }
        }

        if (stringSize > size_t(-1) - headerSize - 1)
        {
            return false;
        }

        const size_t allocationSize = headerSize + stringSize + 1;
        if (!isRangeInBounds(offset, allocationSize, alignof(OffsetString)))
        {
            return false;
        }

        return m_data[size_t(offset) + headerSize + stringSize] == 0;
    }

    bool validateRequiredString(Offset32Ptr<OffsetString> ptr) const
    {
        return !ptr.isNull() && validateString(ptr);
    }

    bool validateStringPtrArray(
        const Offset32Array<Offset32Ptr<OffsetString>>& array,
        bool requireElements) const
    {
        const Offset32Ptr<OffsetString>* strings = nullptr;
        if (!tryGetArrayData(array, strings))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (requireElements && strings[i].isNull())
            {
                return false;
            }

            if (!validateString(strings[i]))
            {
                return false;
            }
        }

        return true;
    }

    bool validateStringPairArray(const Offset32Array<StringPair>& array) const
    {
        const StringPair* pairs = nullptr;
        if (!tryGetArrayData(array, pairs))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (!validateRequiredString(pairs[i].first) || !validateRequiredString(pairs[i].second))
            {
                return false;
            }
        }

        return true;
    }

    bool validateFileState(const FileState& file) const
    {
        // extractFiles() dereferences uniqueName unconditionally inside its
        // pathInfoMap iteration, regardless of whether contents is set.
        if (file.uniqueName.isNull())
        {
            return false;
        }

        return validateString(file.uniqueIdentity) && validateString(file.contents) &&
               validateString(file.canonicalPath) && validateString(file.foundPath) &&
               validateString(file.uniqueName);
    }

    bool validateFilePtr(Offset32Ptr<FileState> filePtr, bool requireFile) const
    {
        const FileState* file = nullptr;
        if (requireFile)
        {
            if (!tryGetRequiredObject(filePtr, file))
            {
                return false;
            }
        }
        else if (!tryGetObject(filePtr, file))
        {
            return false;
        }

        return !file || validateFileState(*file);
    }

    bool validateFilePtrArray(const Offset32Array<Offset32Ptr<FileState>>& array) const
    {
        const Offset32Ptr<FileState>* files = nullptr;
        if (!tryGetArrayData(array, files))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (!validateFilePtr(files[i], true))
            {
                return false;
            }
        }

        return true;
    }

    bool validatePathInfoState(const PathInfoState& pathInfo) const
    {
        return validateFilePtr(pathInfo.file, false);
    }

    bool validateSourceFileState(const SourceFileState& sourceFile) const
    {
        const FileState* file = nullptr;
        if (!tryGetRequiredObject(sourceFile.file, file))
        {
            return false;
        }

        // load() materializes SourceFile from blob; require serialized contents.
        if (file->contents.isNull())
        {
            return false;
        }

        return validateString(sourceFile.foundPath) && validateFileState(*file);
    }

    bool validateSourceFilePtr(Offset32Ptr<SourceFileState> sourceFilePtr) const
    {
        const SourceFileState* sourceFile = nullptr;
        return tryGetRequiredObject(sourceFilePtr, sourceFile) &&
               validateSourceFileState(*sourceFile);
    }

    bool validateSourceFilePtrArray(const Offset32Array<Offset32Ptr<SourceFileState>>& array) const
    {
        const Offset32Ptr<SourceFileState>* sourceFiles = nullptr;
        if (!tryGetArrayData(array, sourceFiles))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (!validateSourceFilePtr(sourceFiles[i]))
            {
                return false;
            }
        }

        return true;
    }

    bool validateOutputStateArray(const Offset32Array<OutputState>& array, uint32_t entryPointCount)
        const
    {
        const OutputState* outputStates = nullptr;
        if (!tryGetArrayData(array, outputStates))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            const OutputState& outputState = outputStates[i];
            if (outputState.entryPointIndex < 0 ||
                uint32_t(outputState.entryPointIndex) >= entryPointCount)
            {
                return false;
            }

            if (!validateString(outputState.outputPath))
            {
                return false;
            }
        }

        return true;
    }

    bool validateTargetRequestArray(
        const Offset32Array<TargetRequestState>& array,
        uint32_t entryPointCount) const
    {
        const TargetRequestState* targetRequests = nullptr;
        if (!tryGetArrayData(array, targetRequests))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (!validateOutputStateArray(targetRequests[i].outputStates, entryPointCount))
            {
                return false;
            }
        }

        return true;
    }

    bool validatePathInfoMap(const Offset32Array<PathAndPathInfo>& array) const
    {
        const PathAndPathInfo* paths = nullptr;
        if (!tryGetArrayData(array, paths))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            if (!validateRequiredString(paths[i].path))
            {
                return false;
            }

            const PathInfoState* pathInfo = nullptr;
            if (!tryGetObject(paths[i].pathInfo, pathInfo))
            {
                return false;
            }

            if (pathInfo && !validatePathInfoState(*pathInfo))
            {
                return false;
            }
        }

        return true;
    }

    bool validateTranslationUnitArray(const Offset32Array<TranslationUnitRequestState>& array) const
    {
        const TranslationUnitRequestState* translationUnits = nullptr;
        if (!tryGetArrayData(array, translationUnits))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            const TranslationUnitRequestState& translationUnit = translationUnits[i];
            if (!validateString(translationUnit.moduleName) ||
                !validateStringPairArray(translationUnit.preprocessorDefinitions) ||
                !validateSourceFilePtrArray(translationUnit.sourceFiles))
            {
                return false;
            }
        }

        return true;
    }

    bool validateEntryPointArray(
        const Offset32Array<EntryPointState>& array,
        uint32_t translationUnitCount) const
    {
        const EntryPointState* entryPoints = nullptr;
        if (!tryGetArrayData(array, entryPoints))
        {
            return false;
        }

        for (uint32_t i = 0; i < array.m_count; ++i)
        {
            const EntryPointState& entryPoint = entryPoints[i];
            if (entryPoint.translationUnitIndex >= translationUnitCount)
            {
                return false;
            }

            if (!validateString(entryPoint.name) ||
                !validateStringPtrArray(entryPoint.specializationArgStrings, false))
            {
                return false;
            }
        }

        return true;
    }

    bool validateRequestState(const RequestState& requestState) const
    {
        const uint32_t entryPointCount = requestState.entryPoints.m_count;
        const uint32_t translationUnitCount = requestState.translationUnits.m_count;

        return validateFilePtrArray(requestState.files) &&
               validateSourceFilePtrArray(requestState.sourceFiles) &&
               validateTargetRequestArray(requestState.targetRequests, entryPointCount) &&
               validateStringPtrArray(requestState.searchPaths, true) &&
               validateStringPairArray(requestState.preprocessorDefinitions) &&
               validatePathInfoMap(requestState.pathInfoMap) &&
               validateTranslationUnitArray(requestState.translationUnits) &&
               validateEntryPointArray(requestState.entryPoints, translationUnitCount);
    }

    const uint8_t* m_data;
    size_t m_dataSize;
};

} // namespace

bool isReproStateValid(const List<uint8_t>& buffer)
{
    return ReproStateValidator(buffer.getBuffer(), buffer.getCount()).validate();
}

} // namespace Slang
