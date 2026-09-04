// slang-ir-obfuscate-loc.cpp
#include "slang-ir-obfuscate-loc.h"

#include "core/slang-castable.h"
#include "core/slang-char-util.h"
#include "core/slang-random-generator.h"
#include "core/slang-stable-hash.h"
#include "slang.h"

namespace Slang
{

namespace
{ // anonymous

struct InstWithLoc
{
    typedef InstWithLoc ThisType;

    SLANG_FORCE_INLINE bool operator<(const ThisType& rhs) const
    {
        return loc.getRaw() < rhs.loc.getRaw();
    }

    IRInst* inst;
    SourceLoc loc;
};

struct LocPair
{
    /// The raw IR instruction loc, which may be a per-invocation macro expansion-range loc.
    /// Used as the identity key to match IR instructions in the apply loop.
    SourceLoc originalLoc;
    /// The definition-file loc for source-map and hash purposes. Normally equals originalLoc,
    /// but for macro-body expansion-range locs it holds the unmapped definition-file loc
    /// (recovered via findSourceViewThroughExpansion) so that findSourceViewRecursively works.
    SourceLoc definitionLoc;
    SourceLoc obfuscatedLoc;
};

} // namespace

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

// We assume the root source manager is the core module
static SourceLoc _getCoreModuleLastLoc(SourceManager* sourceManager)
{
    auto rootManager = sourceManager;
    while (rootManager->getParent())
    {
        rootManager = rootManager->getParent();
    }
    return rootManager->getNextRangeStart();
}

SlangResult obfuscateModuleLocs(IRModule* module, SourceManager* sourceManager)
{
    // There shouldn't be an obfuscated source map set
    SLANG_ASSERT(module->getObfuscatedSourceMap() == nullptr);

    List<InstWithLoc> instWithLocs;

    // Find all of the instructions with source locs
    _findInstsRec(module->getModuleInst(), instWithLocs);
    if (instWithLocs.getCount() == 0)
    {
        // Nothing to do
        return SLANG_OK;
    }

    // Sort them
    instWithLocs.sort();

    // Lets produce a hash, so we can use as a key for random number generation.
    //
    // We could base it on time, or some other random seed. But it would be preferable
    // if it was stable, and compilations of the same module on different machines
    // produce the same hash.
    //
    // Doing so would mean that we could use the obfuscated location ouput to output
    // the origin.

    StableHashCode32 hash{0};

    List<LocPair> locPairs;

    // We want the hash to be stable. One problem is the source locs depend on their order of
    // inclusion. To work around this we are going to hash via offsets, not locs.
    {
        SourceView* sourceView = nullptr;

        const SourceLoc endCoreModuleLoc = _getCoreModuleLastLoc(sourceManager);

        SourceLoc curLoc;
        for (const auto& instWithLoc : instWithLocs)
        {
            if (instWithLoc.loc != curLoc)
            {
                LocPair locPair;
                locPair.originalLoc = instWithLoc.loc;
                // definitionLoc starts equal to originalLoc; it is updated below if the loc is an
                // expansion-range loc that must be unmapped to the definition-file loc.
                locPair.definitionLoc = instWithLoc.loc;
                locPairs.add(locPair);

                // curLoc is the dedup key for the `if` above. It must always be the raw IR
                // instruction loc (never the unmapped loc), because the apply loop compares
                // instWithLoc.loc against pair.originalLoc, which is also the raw IR loc.
                curLoc = instWithLoc.loc;

                // definitionLoc tracks the definition-file loc for SourceView lookups and hash
                // computation. For expansion-range locs (which have no SourceView), definitionLoc
                // is updated to the unmapped loc below; for regular locs it stays equal to curLoc.
                SourceLoc definitionLoc = instWithLoc.loc;

                // Resolve definitionLoc to a concrete SourceView, unmapping through the macro
                // expansion side table first if this is a per-invocation expansion-range loc.
                //
                // This must happen *before* the core-module skip check below, not after. A macro
                // that is both defined and invoked inside the core module (e.g. a helper macro
                // used within hlsl.meta.slang itself) has its body-token locs remapped into an
                // expansion-range loc that itself falls below endCoreModuleLoc, but that raw
                // expansion-range loc has no backing SourceView on its own -- only the unmapped
                // definition-file loc does. The source-map loop further down calls
                // findSourceViewRecursively(pair.definitionLoc) unconditionally on every LocPair,
                // including ones skipped from the hash by the core-module check, so
                // definitionLoc must already be resolved to a real file loc (or the LocPair must
                // never have been added) by the time we get there. Checking core-module-ness
                // against the still-unmapped expansion-range loc, as the code used to do, left
                // definitionLoc pointing at an unresolvable loc for exactly this case, and the
                // source-map loop's SLANG_ASSERT(curView) would fire on it.
                // Tracks whether the lookup below actually switched sourceView to a new file
                // (as opposed to reusing the cached one from a prior loc that fell in the same
                // range). The view's path name only needs to be folded into the hash once per
                // file, not once per loc -- it's constant within a view -- so the nameHash
                // combine below is gated on this flag rather than running unconditionally.
                bool enteredNewSourceView = false;
                if (sourceView == nullptr || !sourceView->getRange().contains(definitionLoc))
                {
                    sourceView = sourceManager->findSourceViewRecursively(definitionLoc);
                    if (sourceView == nullptr)
                    {
                        // The loc may be a per-invocation expansion-range loc produced by
                        // _maybeRemapBodyTokenLoc (a synthetic range with no SourceView).
                        // Walk the macro expansion side table to recover the definition-file loc.
                        SourceLoc unmappedLoc = definitionLoc;
                        sourceView = sourceManager->findSourceViewThroughExpansion(unmappedLoc);
                        if (sourceView != nullptr)
                        {
                            // Store the unmapped loc in definitionLoc (local) and in the LocPair,
                            // so the hash computation and the source-map loop can call
                            // findSourceViewRecursively without another unmap.
                            // Do NOT update curLoc — it is the dedup key for the outer `if`, and
                            // must remain the raw IR expansion-range loc so that the next
                            // instruction with the same loc is correctly identified as a duplicate.
                            definitionLoc = unmappedLoc;
                            locPairs.getLast().definitionLoc = unmappedLoc;
                        }
                        else
                        {
                            // Genuinely unknown loc — skip it from the hash.
                            continue;
                        }
                    }
                    enteredNewSourceView = true;
                }

                // Ignore any core module locs in the hash. definitionLoc is already resolved to
                // a real definition-file loc at this point (see above), so this correctly
                // recognizes a core-module-defined macro's remapped body locs as core-module
                // locs too, not just unremapped ones.
                if (definitionLoc.getRaw() < endCoreModuleLoc.getRaw())
                {
                    continue;
                }

                if (enteredNewSourceView)
                {
                    const auto pathInfo = sourceView->getViewPathInfo();
                    const auto name = pathInfo.getName();
                    const auto nameHash = getStableHashCode32(name.getBuffer(), name.getLength());

                    // Combine the name
                    hash = combineStableHash(hash, nameHash);
                }

                // We *can't* just use the offset to produce the hash, because the source might have
                // different line endings on different platforms (in particular linux/unix-like and
                // windows). So we hash the line number/line offset to work around

                const auto offset = sourceView->getRange().getOffset(definitionLoc);

                const auto sourceFile = sourceView->getSourceFile();
                const auto lineIndex = sourceFile->calcLineIndexFromOffset(offset);
                const auto lineOffset = sourceFile->calcColumnOffset(lineIndex, offset);

                hash = combineStableHash(
                    hash,
                    getStableHashCode32(lineIndex),
                    getStableHashCode32(lineOffset));
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

            // Make it clear this "source" is actually just for obfuscation.
            buf << toSlice("-obfuscated");

            obfusctatedPathInfo = PathInfo::makePath(buf);
        }
    }

    SourceFile* obfuscatedFile =
        sourceManager->createSourceFileWithSize(obfusctatedPathInfo, uniqueLocCount);

    // We put each loc on it's own line. We do this rather than using a single line because
    // it means the `#line` directives can still do something meaningful, since the best resolution
    // they have is a single line.
    {
        List<uint32_t> offsets;
        offsets.setCount(uniqueLocCount + 1);
        for (Index i = 0; i < uniqueLocCount + 1; ++i)
        {
            offsets[i] = uint32_t(i);
        }

        obfuscatedFile->setLineBreakOffsets(offsets.getBuffer(), offsets.getCount());
    }

    // Create the view we are going to use from the obfusctated "file".
    SourceView* obfuscatedView =
        sourceManager->createSourceView(obfuscatedFile, nullptr, SourceLoc());

    const auto obfuscatedRange = obfuscatedView->getRange();

    // Okay now we want to produce a map from these locs to a new source location
    {
        // Create a "bag" and put all of the indices in it.
        List<SourceLoc> bag;

        bag.setCount(uniqueLocCount);

        {
            SourceLoc* dst = bag.getBuffer();
            for (Index i = 0; i < uniqueLocCount; ++i)
            {
                dst[i] = obfuscatedRange.begin + i;
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
    if (const LocPair* curPair = locPairs.getBuffer())
    {
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

    ComPtr<IBoxValue<SourceMap>> boxedSourceMap(new BoxValue<SourceMap>);

    auto sourceMap = boxedSourceMap->getPtr();

    sourceMap->m_file = obfusctatedPathInfo.getName();

    // Set up entries one per line
    List<SourceMap::Entry> entries;
    {
        entries.setCount(uniqueLocCount);
        for (auto& entry : entries)
        {
            entry.init();
        }
    }

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

            // First find the view. pair.definitionLoc is the definition-file loc (unmapped from the
            // expansion-range loc in the hash loop above), so findSourceViewRecursively works.
            if (curView == nullptr || !curView->getRange().contains(pair.definitionLoc))
            {
                curView = sourceManager->findSourceViewRecursively(pair.definitionLoc);
                SLANG_ASSERT(curView);

                // Reset the current view path index, to being unset
                curViewSourceFileIndex = -1;

                // We have to reset, because the path index is for the source manager
                // that holds the view. If the view changes we need to re determine the
                // path string, and index.
                curPathSourceFileIndex = -1;
            }

            // Now get the location
            const auto handleLoc = curView->getHandleLoc(pair.definitionLoc);

            Index sourceFileIndex = -1;

            if (handleLoc.pathHandle == StringSlicePool::Handle(0))
            {
                if (curViewSourceFileIndex < 0)
                {
                    const auto pathInfo = curView->getViewPathInfo();
                    curViewSourceFileIndex =
                        sourceMap->getSourceFileIndex(pathInfo.getName().getUnownedSlice());
                }
                sourceFileIndex = curViewSourceFileIndex;
            }
            else
            {
                if (curPathSourceFileIndex < 0 || handleLoc.pathHandle != curPathHandle)
                {
                    auto viewSourceManager = curView->getSourceManager();
                    const auto filePathSlice =
                        viewSourceManager->getStringSlicePool().getSlice(curPathHandle);

                    // Set the handle
                    curPathHandle = handleLoc.pathHandle;

                    // Get the source file index.
                    curPathSourceFileIndex = sourceMap->getSourceFileIndex(filePathSlice);
                }

                sourceFileIndex = curPathSourceFileIndex;
            }

            // Calculate the line index associated with this loc
            const Index generatedLineIndex = Index(obfuscatedRange.getOffset(pair.obfuscatedLoc));

            // Set it up
            SourceMap::Entry& entry = entries[generatedLineIndex];

            entry.sourceFileIndex = sourceFileIndex;

            // The generated has a line per loc, so the generated column is always 0
            entry.generatedColumn = 0;

            // We need to subtract 1, because handleLoc locations are 1 indexed, but SourceMap
            // entry is 0 indexed.
            entry.sourceColumn = handleLoc.column - 1;
            entry.sourceLine = handleLoc.line - 1;
        }
    }

    // Add all of the entries in line order to the source map
    for (Index i = 0; i < uniqueLocCount; ++i)
    {
        // Advance to the current line.
        sourceMap->advanceToLine(i);
        // Add it to the source map
        sourceMap->addEntry(entries[i]);
    }

    // Associate the sourceMap with the obfuscated file.
    obfuscatedFile->setSourceMap(boxedSourceMap, SourceMapKind::Obfuscated);

    // Set the obfuscated map onto the module
    module->setObfuscatedSourceMap(boxedSourceMap);

    return SLANG_OK;
}

} // namespace Slang
