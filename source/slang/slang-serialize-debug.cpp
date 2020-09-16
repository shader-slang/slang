// slang-serialize-debug.cpp
#include "slang-serialize-debug.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "../core/slang-math.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DebugSerialData !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

size_t DebugSerialData::calcSizeInBytes() const
{
    return SerialListUtil::calcArraySize(m_debugStringTable) +
        SerialListUtil::calcArraySize(m_debugLineInfos) +
        SerialListUtil::calcArraySize(m_debugSourceInfos) +
        SerialListUtil::calcArraySize(m_debugAdjustedLineInfos);
}

void DebugSerialData::clear()
{
    m_debugLineInfos.clear();
    m_debugAdjustedLineInfos.clear();
    m_debugSourceInfos.clear();
    m_debugStringTable.clear();
}


bool DebugSerialData::operator==(const ThisType& rhs) const
{
    return (this == &rhs) ||
        (   SerialListUtil::isEqual(m_debugStringTable, rhs.m_debugStringTable) &&
            SerialListUtil::isEqual(m_debugLineInfos, rhs.m_debugLineInfos) &&
            SerialListUtil::isEqual(m_debugAdjustedLineInfos, rhs.m_debugAdjustedLineInfos) &&
            SerialListUtil::isEqual(m_debugSourceInfos, rhs.m_debugSourceInfos));
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DebugSerialWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

DebugSerialData::SourceLoc DebugSerialWriter::addSourceLoc(SourceLoc sourceLoc)
{
    // If it's not set we can ignore
    if (!sourceLoc.isValid())
    {
        return DebugSerialData::SourceLoc(0);
    }

    // Look up the view it's from
    SourceView* sourceView = m_sourceManager->findSourceView(sourceLoc);
    if (!sourceView)
    {
        // If not found we just ingore 
        return DebugSerialData::SourceLoc(0);
    }

    SourceFile* sourceFile = sourceView->getSourceFile();
    DebugSourceFile* debugSourceFile;
    {
        RefPtr<DebugSourceFile>* ptrDebugSourceFile = m_debugSourceFileMap.TryGetValue(sourceFile);
        if (ptrDebugSourceFile == nullptr)
        {
            const SourceLoc::RawValue baseSourceLoc = m_debugFreeSourceLoc;
            m_debugFreeSourceLoc += SourceLoc::RawValue(sourceView->getRange().getSize() + 1);

            debugSourceFile = new DebugSourceFile(sourceFile, baseSourceLoc);
            m_debugSourceFileMap.Add(sourceFile, debugSourceFile);
        }
        else
        {
            debugSourceFile = *ptrDebugSourceFile;
        }
    }

    // We need to work out the line index

    int offset = sourceView->getRange().getOffset(sourceLoc);
    int lineIndex = sourceFile->calcLineIndexFromOffset(offset);

    DebugSerialData::DebugLineInfo lineInfo;
    lineInfo.m_lineStartOffset = sourceFile->getLineBreakOffsets()[lineIndex];
    lineInfo.m_lineIndex = lineIndex;

    if (!debugSourceFile->hasLineIndex(lineIndex))
    {
        // Add the information about the line        
        int entryIndex = sourceView->findEntryIndex(sourceLoc);
        if (entryIndex < 0)
        {
            debugSourceFile->m_lineInfos.add(lineInfo);
        }
        else
        {
            const auto& entry = sourceView->getEntries()[entryIndex];

            DebugSerialData::DebugAdjustedLineInfo adjustedLineInfo;
            adjustedLineInfo.m_lineInfo = lineInfo;
            adjustedLineInfo.m_pathStringIndex = SerialStringData::kNullStringIndex;

            const auto& pool = sourceView->getSourceManager()->getStringSlicePool();
            SLANG_ASSERT(pool.getStyle() == StringSlicePool::Style::Default);

            if (!pool.isDefaultHandle(entry.m_pathHandle))
            {
                UnownedStringSlice slice = pool.getSlice(entry.m_pathHandle);
                SLANG_ASSERT(slice.getLength() > 0);
                adjustedLineInfo.m_pathStringIndex = DebugSerialData::StringIndex(m_debugStringSlicePool.add(slice));
            }

            adjustedLineInfo.m_adjustedLineIndex = lineIndex + entry.m_lineAdjust;

            debugSourceFile->m_adjustedLineInfos.add(adjustedLineInfo);
        }

        debugSourceFile->setHasLineIndex(lineIndex);
    }

    return DebugSerialData::SourceLoc(debugSourceFile->m_baseSourceLoc + offset);
}

void DebugSerialWriter::write(DebugSerialData* outDebugData)
{
    outDebugData->clear();

    // Okay we can now calculate the final source information

    for (auto& pair : m_debugSourceFileMap)
    {
        DebugSourceFile* debugSourceFile = pair.Value;
        SourceFile* sourceFile = debugSourceFile->m_sourceFile;

        DebugSerialData::DebugSourceInfo sourceInfo;

        sourceInfo.m_numLines = uint32_t(debugSourceFile->m_sourceFile->getLineBreakOffsets().getCount());

        sourceInfo.m_range.m_start = uint32_t(debugSourceFile->m_baseSourceLoc);
        sourceInfo.m_range.m_end = uint32_t(debugSourceFile->m_baseSourceLoc + sourceFile->getContentSize());

        sourceInfo.m_pathIndex = DebugSerialData::StringIndex(m_debugStringSlicePool.add(sourceFile->getPathInfo().foundPath));

        sourceInfo.m_lineInfosStartIndex = uint32_t(outDebugData->m_debugLineInfos.getCount());
        sourceInfo.m_adjustedLineInfosStartIndex = uint32_t(outDebugData->m_debugAdjustedLineInfos.getCount());

        sourceInfo.m_numLineInfos = uint32_t(debugSourceFile->m_lineInfos.getCount());
        sourceInfo.m_numAdjustedLineInfos = uint32_t(debugSourceFile->m_adjustedLineInfos.getCount());

        // Add the line infos
        outDebugData->m_debugLineInfos.addRange(debugSourceFile->m_lineInfos.begin(), debugSourceFile->m_lineInfos.getCount());
        outDebugData->m_debugAdjustedLineInfos.addRange(debugSourceFile->m_adjustedLineInfos.begin(), debugSourceFile->m_adjustedLineInfos.getCount());

        // Add the source info
        outDebugData->m_debugSourceInfos.add(sourceInfo);
    }

    // Convert the string pool
    SerialStringTableUtil::encodeStringTable(m_debugStringSlicePool, outDebugData->m_debugStringTable);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DebugSerialReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Index DebugSerialReader::findViewIndex(DebugSerialData::SourceLoc loc)
{
    for (Index i = 0; i < m_views.getCount(); ++i)
    {
        if (m_views[i].m_range.contains(loc))
        {
            return i;
        }
    }
    return -1;
}


int DebugSerialReader::calcFixSourceLoc(DebugSerialData::SourceLoc loc, DebugSerialData::SourceRange& outRange)
{
    if (m_lastViewIndex < 0 || !m_views[m_lastViewIndex].m_range.contains(loc))
    {
        m_lastViewIndex = findViewIndex(loc);
    }

    if (m_lastViewIndex < 0)
    {
        SLANG_ASSERT(!"Invalid Loc mapping");
        return 0;
    }

    const auto& view = m_views[m_lastViewIndex];

    SLANG_ASSERT(view.m_range.contains(loc));

    outRange = view.m_range;
    return view.m_sourceView->getRange().begin.getRaw() - view.m_range.m_start;
}

SourceLoc DebugSerialReader::getSourceLoc(DebugSerialData::SourceLoc loc)
{
    if (loc != 0)
    {
        if (m_lastViewIndex >= 0)
        {
            const auto& view = m_views[m_lastViewIndex];
            if (view.m_range.contains(loc))
            {
                return view.m_range.getSourceLoc(loc, view.m_sourceView);
            }
        }

        m_lastViewIndex = findViewIndex(loc);
        if (m_lastViewIndex >= 0)
        {
            const auto& view = m_views[m_lastViewIndex];
            return view.m_range.getSourceLoc(loc, view.m_sourceView);
        }
    }
    return SourceLoc();
}

SlangResult DebugSerialReader::read(const DebugSerialData* serialData, SourceManager* sourceManager)
{
    m_views.setCount(0);

    if (!sourceManager || serialData->m_debugSourceInfos.getCount() == 0)
    {
        return SLANG_OK;
    }

    List<UnownedStringSlice> debugStringSlices;
    SerialStringTableUtil::decodeStringTable(serialData->m_debugStringTable.getBuffer(), serialData->m_debugStringTable.getCount(), debugStringSlices);

    // All of the strings are placed in the manager (and its StringSlicePool) where the SourceView and SourceFile are constructed from
    List<StringSlicePool::Handle> stringMap;
    SerialStringTableUtil::calcStringSlicePoolMap(debugStringSlices, sourceManager->getStringSlicePool(), stringMap);

    // Construct the source files
    const Index numSourceFiles = serialData->m_debugSourceInfos.getCount();

    // These hold the views (and SourceFile as there is only one SourceFile per view) in the same order as the sourceInfos
    m_views.setCount(numSourceFiles);

    for (Index i = 0; i < numSourceFiles; ++i)
    {
        const auto& srcSourceInfo = serialData->m_debugSourceInfos[i];

        PathInfo pathInfo;
        pathInfo.type = PathInfo::Type::FoundPath;
        pathInfo.foundPath = debugStringSlices[UInt(srcSourceInfo.m_pathIndex)];

        SourceFile* sourceFile = sourceManager->createSourceFileWithSize(pathInfo, srcSourceInfo.m_range.getCount());
        SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr);

        // We need to accumulate all line numbers, for this source file, both adjusted and unadjusted
        List<DebugSerialData::DebugLineInfo> lineInfos;
        // Add the adjusted lines
        {
            lineInfos.setCount(srcSourceInfo.m_numAdjustedLineInfos);
            const DebugSerialData::DebugAdjustedLineInfo* srcAdjustedLineInfos = serialData->m_debugAdjustedLineInfos.getBuffer() + srcSourceInfo.m_adjustedLineInfosStartIndex;
            const int numAdjustedLines = int(srcSourceInfo.m_numAdjustedLineInfos);
            for (int j = 0; j < numAdjustedLines; ++j)
            {
                lineInfos[j] = srcAdjustedLineInfos[j].m_lineInfo;
            }
        }
        // Add regular lines
        lineInfos.addRange(serialData->m_debugLineInfos.getBuffer() + srcSourceInfo.m_lineInfosStartIndex, srcSourceInfo.m_numLineInfos);

        // Put in sourceloc order
        lineInfos.sort();

        List<uint32_t> lineBreakOffsets;

        // We can now set up the line breaks array
        const int numLines = int(srcSourceInfo.m_numLines);
        lineBreakOffsets.setCount(numLines);

        {
            const Index numLineInfos = lineInfos.getCount();
            Index lineIndex = 0;

            // Every line up and including should hold the same offset
            for (Index lineInfoIndex = 0; lineInfoIndex < numLineInfos; ++lineInfoIndex)
            {
                const auto& lineInfo = lineInfos[lineInfoIndex];

                const uint32_t offset = lineInfo.m_lineStartOffset;
                SLANG_ASSERT(offset > 0);
                const int finishIndex = int(lineInfo.m_lineIndex);

                SLANG_ASSERT(finishIndex < numLines);

                for (; lineIndex < finishIndex; ++lineIndex)
                {
                    lineBreakOffsets[lineIndex] = offset - 1;
                }
                lineBreakOffsets[lineIndex] = offset;
                lineIndex++;
            }

            // Do the remaining lines
            {
                const uint32_t endOffset = srcSourceInfo.m_range.getCount();
                for (; lineIndex < numLines; ++lineIndex)
                {
                    lineBreakOffsets[lineIndex] = endOffset;
                }
            }
        }

        sourceFile->setLineBreakOffsets(lineBreakOffsets.getBuffer(), lineBreakOffsets.getCount());

        if (srcSourceInfo.m_numAdjustedLineInfos)
        {
            List<DebugSerialData::DebugAdjustedLineInfo> adjustedLineInfos;

            int numEntries = int(srcSourceInfo.m_numAdjustedLineInfos);

            adjustedLineInfos.addRange(serialData->m_debugAdjustedLineInfos.getBuffer() + srcSourceInfo.m_adjustedLineInfosStartIndex, numEntries);
            adjustedLineInfos.sort();

            // Work out the views adjustments, and place in dstEntries
            List<SourceView::Entry> dstEntries;
            dstEntries.setCount(numEntries);

            const uint32_t sourceLocOffset = uint32_t(sourceView->getRange().begin.getRaw());

            for (int j = 0; j < numEntries; ++j)
            {
                const auto& srcEntry = adjustedLineInfos[j];
                auto& dstEntry = dstEntries[j];

                dstEntry.m_pathHandle = stringMap[int(srcEntry.m_pathStringIndex)];
                dstEntry.m_startLoc = SourceLoc::fromRaw(srcEntry.m_lineInfo.m_lineStartOffset + sourceLocOffset);
                dstEntry.m_lineAdjust = int32_t(srcEntry.m_adjustedLineIndex) - int32_t(srcEntry.m_lineInfo.m_lineIndex);
            }

            // Set the adjustments on the view
            sourceView->setEntries(dstEntries.getBuffer(), dstEntries.getCount());
        }

        // Set the view and the source range
        View& view = m_views[i];
        view.m_sourceView = sourceView;
        view.m_range = srcSourceInfo.m_range;
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DebugSerialData !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */Result DebugSerialData::writeContainer(SerialCompressionType moduleCompressionType, RiffContainer* container)
{    
    RiffContainer::ScopeChunk debugChunkScope(container, RiffContainer::Chunk::Kind::List, DebugSerialData::kDebugFourCc);

    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayUncompressedChunk(DebugSerialData::kDebugStringFourCc, m_debugStringTable, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayUncompressedChunk(DebugSerialData::kDebugLineInfoFourCc, m_debugLineInfos, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayUncompressedChunk(DebugSerialData::kDebugAdjustedLineInfoFourCc, m_debugAdjustedLineInfos, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayChunk(moduleCompressionType, DebugSerialData::kDebugSourceInfoFourCc, m_debugSourceInfos, container));

    return SLANG_OK;
}

/* static */Result DebugSerialData::readContainer(SerialCompressionType moduleCompressionType, RiffContainer::ListChunk* listChunk)
{
    SLANG_ASSERT(listChunk->getSubType() == DebugSerialData::kDebugFourCc);

    clear();
    for (RiffContainer::Chunk* chunk = listChunk->m_containedChunks; chunk; chunk = chunk->m_next)
    {
        RiffContainer::DataChunk* dataChunk = as<RiffContainer::DataChunk>(chunk);
        if (!dataChunk)
        {
            continue;
        }

        switch (dataChunk->m_fourCC)
        {
            case DebugSerialData::kDebugStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayUncompressedChunk(dataChunk, m_debugStringTable));
                break;
            }
            case DebugSerialData::kDebugLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayUncompressedChunk(dataChunk, m_debugLineInfos));
                break;
            }
            case DebugSerialData::kDebugAdjustedLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayUncompressedChunk(dataChunk, m_debugAdjustedLineInfos));
                break;
            }
            case DebugSerialData::kDebugSourceInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayChunk(moduleCompressionType, dataChunk, m_debugSourceInfos));
                break;
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
