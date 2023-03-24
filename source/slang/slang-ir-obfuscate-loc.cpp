// slang-ir-obfuscate-loc.cpp
#include "slang-ir-obfuscate-loc.h"

#include "../../slang.h"

#include "../core/slang-random-generator.h"
#include "../core/slang-hash.h"
#include "../core/slang-char-util.h"

namespace Slang
{

namespace { // anonymous

struct InstWithLoc
{
    typedef InstWithLoc ThisType;

    SLANG_FORCE_INLINE bool operator<(const ThisType& rhs) const { return loc.getRaw() < rhs.loc.getRaw(); }

    IRInst* inst;
    SourceLoc loc;
};

struct LocPair
{
    SourceLoc originalLoc;
    SourceLoc obfuscatedLoc;
};

} // anonymous

static void _findInstsRec(IRInst* inst, List<InstWithLoc>& out)
{
    if (inst->sourceLoc.isValid())
    {
        InstWithLoc instWithLoc;
        instWithLoc.inst = inst;
        instWithLoc.loc = inst->sourceLoc;
        out.add(instWithLoc);
    }

    for (IRInst* child : inst->getModifiableChildren())
    {
        _findInstsRec(child, out);
    }
}

SlangResult obfuscateModuleLocs(IRModule* module, SourceManager* sourceManager)
{
    // There shouldn't be an obfuscated source map set
    SLANG_ASSERT(module->getObfuscatedSourceMap() == nullptr);

    List<InstWithLoc> instWithLocs;

    // Find all of the instructions with source locs
    _findInstsRec(module->getModuleInst(), instWithLocs);

    // Sort them
    instWithLocs.sort();
        
    // Lets produce a hash, so we can use as a key for random number generation.
    // We could base it on time, or some other thing as there is no requirement for 
    // stability or consistency.
    // We use a hash because it avoids issues around clocks, and availability of a clock
    // as a good source of entropy.
    //
    // An argument *could* be made to generate the name via some mechanism that uniquely identified the 
    // combination of flags, options, files, names that identified the compilation, but that is 
    // not easily achieved.
    HashCode hash = 0;

    List<LocPair> locPairs;

    // We want the hash to be stable. One problem is the source locs depend on their order of inclusion.
    // To work around this we are going 
    {
        SourceView* sourceView = nullptr;

        SourceLoc curLoc;
        for (const auto& instWithLoc : instWithLocs)
        {
            if (instWithLoc.loc != curLoc)
            {
                LocPair locPair;
                locPair.originalLoc = instWithLoc.loc;
                locPairs.add(locPair);

                // This is the current loc
                curLoc = instWithLoc.loc;
            
                // If the loc isn't in the view, lookup the view it is in
                if (sourceView == nullptr || 
                    !sourceView->getRange().contains(curLoc))
                {
                    sourceView = sourceManager->findSourceViewRecursively(curLoc);
                    SLANG_ASSERT(sourceView);

                    // Combine the name
                    hash = combineHash(hash, getHashCode(sourceView->getViewPathInfo().getName().getUnownedSlice()));
                }
                SLANG_ASSERT(sourceView);

                // We combine the *offset* which is stable
                hash = combineHash(hash, getHashCode(sourceView->getRange().getOffset(curLoc)));
            }
        }
    }

    const Count uniqueLocCount = locPairs.getCount();

    // We need a seed to make this random on each run
    const uint32_t randomSeed = uint32_t(hash);
    RefPtr<RandomGenerator> rand = RandomGenerator::create(randomSeed);

    // We want a random unique name because we could have multiple obfuscated modules
    // and we need to identify each

    PathInfo obfusctatedPathInfo;

    {
        // We need a pathInfo to *identify* this modules obfuscated locs.
        // We are going to use a random number, seeded from the hash to do this.
        // Turning the number as hex as the name.
        {
            StringBuilder buf;

            uint8_t data[4];
            rand->nextData(data, sizeof(data));

            const Count charsCount = SLANG_COUNT_OF(data) * 2;

            char* dst = buf.prepareForAppend(charsCount);

            for (Index i = 0; i < SLANG_COUNT_OF(data); ++i)
            {
                dst[i * 2 + 0] = CharUtil::getHexChar(data[i] & 0xf);
                dst[i * 2 + 1] = CharUtil::getHexChar(data[i] >> 4);
            }
            buf.appendInPlace(dst, charsCount);
            obfusctatedPathInfo = PathInfo::makePath(buf);
        }
    }

    SourceFile* obfuscatedFile = sourceManager->createSourceFileWithSize(obfusctatedPathInfo, uniqueLocCount);

    // We have only one line for all locs, just set up that way...
    {
        const uint32_t offsets[2] = { 0, uint32_t(uniqueLocCount) };
        obfuscatedFile->setLineBreakOffsets(offsets, SLANG_COUNT_OF(offsets));
    }
    
    // Create the view we are going to use from the obfusctated "file".
    SourceView* obfuscatedView = sourceManager->createSourceView(obfuscatedFile, nullptr, SourceLoc());

    // Okay now we want to produce a map from these locs to a new source location
    {
        // Create a "bag" and put all of the indices in it.
        List<SourceLoc> bag;

        bag.setCount(uniqueLocCount);

        const SourceLoc baseLoc = obfuscatedView->getRange().begin;

        {
            SourceLoc* dst = bag.getBuffer();
            for (Index i = 0; i < uniqueLocCount; ++i)
            {
                dst[i] = baseLoc + i;
            }
        }

        // Pull the indices randomly out of the bag to create the map
        for (auto& pair : locPairs)
        {
            // Find an index in the bag
            const Index bagIndex = rand->nextInt32InRange(0, int32_t(bag.getCount()));
            // Set in the map
            pair.obfuscatedLoc = bag[bagIndex];
            // Remove from the bag
            bag.fastRemoveAt(bagIndex);
        }
    }

    // We can now just set all the new locs in the instructions
    {
        const LocPair* curPair = locPairs.getBuffer();
        LocPair pair = *curPair;

        for (const auto& instWithLoc : instWithLocs)
        {
            auto inst = instWithLoc.inst;

            if (instWithLoc.loc != pair.originalLoc)
            {
                SLANG_ASSERT(curPair < locPairs.end());
                curPair++;
                pair = *curPair;
            }
            SLANG_ASSERT(pair.originalLoc == instWithLoc.loc);

            // Set the loc
            inst->sourceLoc = pair.obfuscatedLoc;
        }
    }

    // We can now create a map. The locs are in order in entries, so that should make lookup easier.
    // This doesn't "leak" anything as the obfuscated loc map is not distributed.

    RefPtr<SourceMap> sourceMap = new SourceMap;
    sourceMap->m_file = obfusctatedPathInfo.getName();

    // Make sure we have line 0.
    // We only end up with one line in the obfuscated map.
    sourceMap->advanceToLine(0);

    {        
        // Current view, with cached "View" based sourceFileIndex
        SourceView* curView = nullptr;
        Index curViewSourceFileIndex = -1;

        // Current handle, and store cached index in curPathSourceFileIndex
        StringSlicePool::Handle curPathHandle = StringSlicePool::Handle(0);
        Index curPathSourceFileIndex = -1;

        for (Index i = 0; i < uniqueLocCount; ++i)
        {
            const auto& pair = locPairs[i];


            // First find the view
            if (curView == nullptr || 
                !curView->getRange().contains(pair.originalLoc))
            {
                curView = sourceManager->findSourceViewRecursively(pair.originalLoc);
                SLANG_ASSERT(curView);

                // Reset the current view path index, to being unset
                curViewSourceFileIndex = -1;

                // We have to reset, because the path index is for the source manager
                // that holds the view. If the view changes we need to re determine the 
                // path string, and index.
                curPathSourceFileIndex = -1;
            }
               
            // Now get the location
            const auto handleLoc = curView->getHandleLoc(pair.originalLoc);

            Index sourceFileIndex = -1;

            if (handleLoc.pathHandle == StringSlicePool::Handle(0))
            {
                if (curViewSourceFileIndex < 0)
                {
                    const auto pathInfo = curView->getViewPathInfo();
                    curViewSourceFileIndex = sourceMap->getSourceFileIndex(pathInfo.getName().getUnownedSlice());
                }
                sourceFileIndex = curViewSourceFileIndex;
            }
            else
            {
                if (curPathSourceFileIndex < 0 || 
                    handleLoc.pathHandle != curPathHandle)
                {
                    auto viewSourceManager = curView->getSourceManager();
                    const auto filePathSlice = viewSourceManager->getStringSlicePool().getSlice(curPathHandle);

                    // Set the handle
                    curPathHandle = handleLoc.pathHandle;

                    // Get the source file index.
                    curPathSourceFileIndex = sourceMap->getSourceFileIndex(filePathSlice);
                }

                sourceFileIndex = curPathSourceFileIndex;
            }

            // Create the entry
            SourceMap::Entry entry;
            entry.init();

            entry.sourceFileIndex = sourceFileIndex;
  
            // i is the generated column
            entry.generatedColumn = i;

            entry.sourceColumn = handleLoc.column - 1;
            entry.sourceLine = handleLoc.line - 1;

            // Add it to the source map
            sourceMap->addEntry(entry);
        }
    }

    module->setObfuscatedSourceMap(sourceMap);

    return SLANG_OK;
}

} // namespace Slang
