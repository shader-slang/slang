// slang-ir-serialize.cpp
#include "slang-ir-serialize.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "slang-ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

// Needed for linkage with some compilers
/* static */ const IRSerialData::StringIndex IRSerialData::kNullStringIndex;
/* static */ const IRSerialData::StringIndex IRSerialData::kEmptyStringIndex;

/* Note that an IRInst can be derived from, but when it derived from it's new members are IRUse variables, and they in 
effect alias over the operands - and reflected in the operand count. There _could_ be other members after these IRUse 
variables, but only a few types include extra data, and these do not have any operands:

* IRConstant        - Needs special-case handling
* IRModuleInst      - Presumably we can just set to the module pointer on reconstruction

Note! That on an IRInst there is an IRType* variable (accessed as getFullType()). As it stands it may NOT actually point 
to an IRType derived type. Its 'ok' as long as it's an instruction that can be used in the place of the type. So this code does not 
bother to check if it's correct, and just casts it.
*/

/* static */const IRSerialData::PayloadInfo IRSerialData::s_payloadInfos[int(Inst::PayloadType::CountOf)] = 
{
    { 0, 0 },   // Empty
    { 1, 0 },   // Operand_1
    { 2, 0 },   // Operand_2
    { 1, 0 },   // OperandAndUInt32,
    { 0, 0 },   // OperandExternal - This isn't correct, Operand has to be specially handled
    { 0, 1 },   // String_1,              
    { 0, 2 },   // String_2,              
    { 0, 0 },   // UInt32,               
    { 0, 0 },   // Float64,
    { 0, 0 }    // Int64,
};

static bool isTextureTypeBase(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstTextureTypeBase && op <= kIROp_LastTextureTypeBase;
}

static bool isConstant(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstConstant && op <= kIROp_LastConstant;
}

struct PrefixString;

namespace { // anonymous

struct CharReader
{
    char operator()(int pos) const { SLANG_UNUSED(pos); return *m_pos++; }
    CharReader(const char* pos) :m_pos(pos) {}
    mutable const char* m_pos;
};

} // anonymous

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringRepresentationCache !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

StringRepresentationCache::StringRepresentationCache():
    m_stringTable(nullptr),
    m_namePool(nullptr),
    m_scopeManager(nullptr)
{
}

void StringRepresentationCache::init(const List<char>* stringTable, NamePool* namePool, ObjectScopeManager* scopeManager)
{
    m_stringTable = stringTable;
    m_namePool = namePool;
    m_scopeManager = scopeManager;

    // Decode the table
    m_entries.setCount(StringSlicePool::kNumDefaultHandles);
    SLANG_COMPILE_TIME_ASSERT(StringSlicePool::kNumDefaultHandles == 2);

    {
        Entry& entry = m_entries[0];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
        entry.m_object = nullptr;
    }
    {
        Entry& entry = m_entries[1];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
        entry.m_object = nullptr;
    }

    {
        const char* start = stringTable->begin();
        const char* cur = start;
        const char* end = stringTable->end();

        while (cur < end)
        {
            CharReader reader(cur);
            const int len = GetUnicodePointFromUTF8(reader);

            Entry entry;
            entry.m_startIndex = uint32_t(reader.m_pos - start);
            entry.m_numChars = len;
            entry.m_object = nullptr;

            m_entries.add(entry);

            cur = reader.m_pos + len;
        }
    }

    m_entries.compress();
}

Name* StringRepresentationCache::getName(Handle handle)
{
    if (handle == StringSlicePool::kNullHandle)
    {
        return nullptr;
    }

    Entry& entry = m_entries[int(handle)];
    if (entry.m_object)
    {
        Name* name = dynamicCast<Name>(entry.m_object);
        if (name)
        {
            return name;
        }
        StringRepresentation* stringRep = static_cast<StringRepresentation*>(entry.m_object);
        // Promote it to a name
        name = m_namePool->getName(String(stringRep));
        entry.m_object = name;
        return name;
    }

    Name* name = m_namePool->getName(String(getStringSlice(handle)));
    entry.m_object = name;
    return name;
}

String StringRepresentationCache::getString(Handle handle)
{
    return String(getStringRepresentation(handle));
}

UnownedStringSlice StringRepresentationCache::getStringSlice(Handle handle) const
{
    const Entry& entry = m_entries[int(handle)];
    const char* start = m_stringTable->begin();

    return UnownedStringSlice(start + entry.m_startIndex, int(entry.m_numChars));
}

StringRepresentation* StringRepresentationCache::getStringRepresentation(Handle handle)
{
    if (handle == StringSlicePool::kNullHandle || handle == StringSlicePool::kEmptyHandle)
    {
        return nullptr;
    }

    Entry& entry = m_entries[int(handle)];
    if (entry.m_object)
    {
        Name* name = dynamicCast<Name>(entry.m_object);
        if (name)
        {
            return name->text.getStringRepresentation();
        }
        return static_cast<StringRepresentation*>(entry.m_object);
    }

    const UnownedStringSlice slice = getStringSlice(handle);
    const UInt size = slice.size();

    StringRepresentation* stringRep = StringRepresentation::createWithCapacityAndLength(size, size);
    memcpy(stringRep->getData(), slice.begin(), size);
    entry.m_object = stringRep;

    // Keep the StringRepresentation in scope
    m_scopeManager->add(stringRep);
    
    return stringRep;
}

char* StringRepresentationCache::getCStr(Handle handle)
{
    // It turns out StringRepresentation is always 0 terminated, so can just use that
    StringRepresentation* rep = getStringRepresentation(handle);
    return rep->getData();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialStringTableUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void SerialStringTableUtil::encodeStringTable(const StringSlicePool& pool, List<char>& stringTable)
{
    // Skip the default handles -> nothing is encoded via them
    return encodeStringTable(pool.getSlices().begin() + StringSlicePool::kNumDefaultHandles, pool.getNumSlices() - StringSlicePool::kNumDefaultHandles, stringTable);
}
    
/* static */void SerialStringTableUtil::encodeStringTable(const UnownedStringSlice* slices, size_t numSlices, List<char>& stringTable)
{
    stringTable.clear();
    for (size_t i = 0; i < numSlices; ++i)
    {
        const UnownedStringSlice slice = slices[i];
        const int len = int(slice.size());
        
        // We need to write into the the string array
        char prefixBytes[6];
        const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
        const Index baseIndex = stringTable.getCount();

        stringTable.setCount(baseIndex + numPrefixBytes + len);

        char* dst = stringTable.begin() + baseIndex;

        memcpy(dst, prefixBytes, numPrefixBytes);
        memcpy(dst + numPrefixBytes, slice.begin(), len);   
    }
}

/* static */void SerialStringTableUtil::appendDecodedStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    const char* start = stringTable.begin();
    const char* cur = start;
    const char* end = stringTable.end();

    while (cur < end)
    {
        CharReader reader(cur);
        const int len = GetUnicodePointFromUTF8(reader);
        slicesOut.add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

/* static */void SerialStringTableUtil::decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.setCount(2);
    slicesOut[0] = UnownedStringSlice(nullptr, size_t(0));
    slicesOut[1] = UnownedStringSlice("", size_t(0));

    appendDecodedStringTable(stringTable, slicesOut);
}

/* static */void SerialStringTableUtil::calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMapOut)
{
    SLANG_ASSERT(slices.getCount() >= StringSlicePool::kNumDefaultHandles);
    SLANG_ASSERT(slices[int(StringSlicePool::kNullHandle)] == "" && slices[int(StringSlicePool::kNullHandle)].begin() == nullptr);
    SLANG_ASSERT(slices[int(StringSlicePool::kEmptyHandle)] == "");

    indexMapOut.setCount(slices.getCount());
    // Set up all of the defaults
    for (int i = 0; i < StringSlicePool::kNumDefaultHandles; ++i)
    {
        indexMapOut[i] = StringSlicePool::Handle(i);
    }

    const Index numSlices = slices.getCount();
    for (Index i = StringSlicePool::kNumDefaultHandles; i < numSlices ; ++i)
    {
        indexMapOut[i] = pool.add(slices[i]);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialData !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

template<typename T>
static size_t _calcArraySize(const List<T>& list)
{
    return list.getCount() * sizeof(T);
}

size_t IRSerialData::calcSizeInBytes() const
{
    return
        _calcArraySize(m_insts) +
        _calcArraySize(m_childRuns) +
        _calcArraySize(m_externalOperands) +
        _calcArraySize(m_stringTable) +
        /* Raw source locs */
        _calcArraySize(m_rawSourceLocs) +
        /* Debug */
        _calcArraySize(m_debugStringTable) +
        _calcArraySize(m_debugLineInfos) +
        _calcArraySize(m_debugSourceInfos) +
        _calcArraySize(m_debugAdjustedLineInfos) +
        _calcArraySize(m_debugSourceLocRuns);
}

IRSerialData::IRSerialData()
{
    clear();
}

void IRSerialData::clear()
{
    // First Instruction is null
    m_insts.setCount(1);
    memset(&m_insts[0], 0, sizeof(Inst));

    m_childRuns.clear();
    m_externalOperands.clear();
    m_rawSourceLocs.clear();

    m_stringTable.clear();
    
    // Debug data
    m_debugLineInfos.clear();
    m_debugAdjustedLineInfos.clear();
    m_debugSourceInfos.clear();
    m_debugSourceLocRuns.clear();
    m_debugStringTable.clear();
}

template <typename T>
static bool _isEqual(const List<T>& aIn, const List<T>& bIn)
{
    if (aIn.getCount() != bIn.getCount())
    {
        return false;
    }

    size_t size = size_t(aIn.getCount());

    const T* a = aIn.begin();
    const T* b = bIn.begin();

    if (a == b)
    {
        return true;
    }

    for (size_t i = 0; i < size; ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }

    return true;
}

bool IRSerialData::operator==(const ThisType& rhs) const
{
    return (this == &rhs) ||
        (_isEqual(m_insts, rhs.m_insts) &&
        _isEqual(m_childRuns, rhs.m_childRuns) &&
        _isEqual(m_externalOperands, rhs.m_externalOperands) &&
        _isEqual(m_rawSourceLocs, rhs.m_rawSourceLocs) &&
        _isEqual(m_stringTable, rhs.m_stringTable) &&
        /* Debug */
        _isEqual(m_debugStringTable, rhs.m_debugStringTable) &&
        _isEqual(m_debugLineInfos, rhs.m_debugLineInfos) &&
        _isEqual(m_debugAdjustedLineInfos, rhs.m_debugAdjustedLineInfos) &&
        _isEqual(m_debugSourceInfos, rhs.m_debugSourceInfos) &&
        _isEqual(m_debugSourceLocRuns, rhs.m_debugSourceLocRuns));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void IRSerialWriter::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.ContainsKey(inst));

    // Add to the map
    m_instMap.Add(inst, Ser::InstIndex(m_insts.getCount()));
    m_insts.add(inst);
}

#if 0
// Find a view index that matches the view by file (and perhaps other characteristics in the future)
static int _findSourceViewIndex(const List<SourceView*>& viewsIn, SourceView* view)
{
    const int numViews = int(viewsIn.Count());
    SourceView*const* views = viewsIn.begin();
    
    SourceFile* sourceFile = view->getSourceFile();

    for (int i = 0; i < numViews; ++i)
    {
        SourceView* curView = views[i];
        // For now we just match on source file
        if (curView->getSourceFile() == sourceFile)
        {
            // It's a hit
            return i;
        }
    }
    return -1;
}
#endif

void IRSerialWriter::_addDebugSourceLocRun(SourceLoc sourceLoc, uint32_t startInstIndex, uint32_t numInsts)
{
    SourceView* sourceView = m_sourceManager->findSourceView(sourceLoc);
    if (!sourceView)
    {
        return;
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

    IRSerialData::DebugLineInfo lineInfo;
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

            IRSerialData::DebugAdjustedLineInfo adjustedLineInfo;
            adjustedLineInfo.m_lineInfo = lineInfo;
            adjustedLineInfo.m_pathStringIndex = Ser::kNullStringIndex;

            if (StringSlicePool::hasContents(entry.m_pathHandle))
            {
                UnownedStringSlice slice = sourceView->getSourceManager()->getStringSlicePool().getSlice(entry.m_pathHandle);
                SLANG_ASSERT(slice.size() > 0);
                adjustedLineInfo.m_pathStringIndex = Ser::StringIndex(m_debugStringSlicePool.add(slice));
            }

            adjustedLineInfo.m_adjustedLineIndex = lineIndex + entry.m_lineAdjust;

            debugSourceFile->m_adjustedLineInfos.add(adjustedLineInfo);
        }

        debugSourceFile->setHasLineIndex(lineIndex);
    }

    // Add the run
    IRSerialData::SourceLocRun sourceLocRun;
    sourceLocRun.m_numInst = numInsts;
    sourceLocRun.m_startInstIndex = IRSerialData::InstIndex(startInstIndex);
    sourceLocRun.m_sourceLoc = uint32_t(debugSourceFile->m_baseSourceLoc + offset);

    m_serialData->m_debugSourceLocRuns.add(sourceLocRun);
}

Result IRSerialWriter::_calcDebugInfo()
{
    // We need to find the unique source Locs
    // We are not going to store SourceLocs directly, because there may be multiple views mapping down to
    // the same underlying source file

    // First find all the unique locs
    struct InstLoc
    {
        typedef InstLoc ThisType;

        SLANG_FORCE_INLINE bool operator<(const ThisType& rhs) const { return sourceLoc < rhs.sourceLoc || (sourceLoc == rhs.sourceLoc && instIndex < rhs.instIndex); }

        uint32_t instIndex;
        uint32_t sourceLoc;
    };

    // Find all of the source locations and their associated instructions
    List<InstLoc> instLocs;
    const Index numInsts = m_insts.getCount();
    for (Index i = 1; i < numInsts; i++)
    {
        IRInst* srcInst = m_insts[i];
        if (!srcInst->sourceLoc.isValid())
        {
            continue;
        }
        InstLoc instLoc;
        instLoc.instIndex = uint32_t(i);
        instLoc.sourceLoc = uint32_t(srcInst->sourceLoc.getRaw());
        instLocs.add(instLoc);
    }

    // Sort them
    instLocs.sort();
    m_debugFreeSourceLoc = 1;

    // Look for runs
    const InstLoc* startInstLoc = instLocs.begin();
    const InstLoc* endInstLoc = instLocs.end();

    while (startInstLoc < endInstLoc)
    {
        const uint32_t startSourceLoc = startInstLoc->sourceLoc;
        
        // Find the run with the same source loc

        const InstLoc* curInstLoc = startInstLoc + 1;
        uint32_t curInstIndex = startInstLoc->instIndex + 1;

        // Find the run size with same source loc and run of instruction indices
        for (; curInstLoc < endInstLoc && curInstLoc->sourceLoc == startSourceLoc && curInstLoc->instIndex == curInstIndex; ++curInstLoc, ++curInstIndex)
        {
        }

        // Try adding the run
        _addDebugSourceLocRun(SourceLoc::fromRaw(startSourceLoc), startInstLoc->instIndex, curInstIndex - startInstLoc->instIndex);

        // Next
        startInstLoc = curInstLoc;
    }

    // Okay we can now calculate the final source information

    for (auto& pair : m_debugSourceFileMap)
    {
        DebugSourceFile* debugSourceFile = pair.Value;
        SourceFile* sourceFile = debugSourceFile->m_sourceFile;

        IRSerialData::DebugSourceInfo sourceInfo;

        sourceInfo.m_numLines = uint32_t(debugSourceFile->m_sourceFile->getLineBreakOffsets().getCount());

        sourceInfo.m_startSourceLoc = uint32_t(debugSourceFile->m_baseSourceLoc);
        sourceInfo.m_endSourceLoc = uint32_t(debugSourceFile->m_baseSourceLoc + sourceFile->getContentSize());

        sourceInfo.m_pathIndex = Ser::StringIndex(m_debugStringSlicePool.add(sourceFile->getPathInfo().foundPath));

        sourceInfo.m_lineInfosStartIndex = uint32_t(m_serialData->m_debugLineInfos.getCount());
        sourceInfo.m_adjustedLineInfosStartIndex = uint32_t(m_serialData->m_debugAdjustedLineInfos.getCount());

        sourceInfo.m_numLineInfos = uint32_t(debugSourceFile->m_lineInfos.getCount());
        sourceInfo.m_numAdjustedLineInfos = uint32_t(debugSourceFile->m_adjustedLineInfos.getCount());

        // Add the line infos
        m_serialData->m_debugLineInfos.addRange(debugSourceFile->m_lineInfos.begin(), debugSourceFile->m_lineInfos.getCount());
        m_serialData->m_debugAdjustedLineInfos.addRange(debugSourceFile->m_adjustedLineInfos.begin(), debugSourceFile->m_adjustedLineInfos.getCount());

        // Add the source info
        m_serialData->m_debugSourceInfos.add(sourceInfo);
    }

    // Convert the string pool
    SerialStringTableUtil::encodeStringTable(m_debugStringSlicePool, m_serialData->m_debugStringTable);

    return SLANG_OK;
}

Result IRSerialWriter::write(IRModule* module, SourceManager* sourceManager, OptionFlags options, IRSerialData* serialData)
{
    typedef Ser::Inst::PayloadType PayloadType;

    m_sourceManager = sourceManager;
    m_serialData = serialData;

    serialData->clear();

    // We reserve 0 for null
    m_insts.clear();
    m_insts.add(nullptr);

    // Reset
    m_instMap.Clear();
    m_decorations.clear();
    
    // Stack for parentInst
    List<IRInst*> parentInstStack;
  
    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.add(moduleInst);

    // Add to the map
    _addInstruction(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.getCount())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRInst* parentInst = parentInstStack.getLast();
        parentInstStack.removeLast();
        SLANG_ASSERT(m_instMap.ContainsKey(parentInst));

        // Okay we go through each of the children in order. If they are IRInstParent derived, we add to stack to process later 
        // cos we want breadth first so the order of children is the same as their index order, meaning we don't need to store explicit indices
        const Ser::InstIndex startChildInstIndex = Ser::InstIndex(m_insts.getCount());
        
        IRInstListBase childrenList = parentInst->getDecorationsAndChildren();
        for (IRInst* child : childrenList)
        {
            // This instruction can't be in the map...
            SLANG_ASSERT(!m_instMap.ContainsKey(child));

            _addInstruction(child);
            
            parentInstStack.add(child);
        }

        // If it had any children, then store the information about it
        if (Ser::InstIndex(m_insts.getCount()) != startChildInstIndex)
        {
            Ser::InstRun run;
            run.m_parentIndex = m_instMap[parentInst];
            run.m_startInstIndex = startChildInstIndex;
            run.m_numChildren = Ser::SizeType(m_insts.getCount() - int(startChildInstIndex));

            m_serialData->m_childRuns.add(run);
        }
    }

#if 0
    {
        List<IRInst*> workInsts;
        calcInstructionList(module, workInsts);
        SLANG_ASSERT(workInsts.Count() == m_insts.Count());
        for (UInt i = 0; i < workInsts.Count(); ++i)
        {
            SLANG_ASSERT(workInsts[i] == m_insts[i]);
        }
    }
#endif

    // Set to the right size
    m_serialData->m_insts.setCount(m_insts.getCount());
    // Clear all instructions
    memset(m_serialData->m_insts.begin(), 0, sizeof(Ser::Inst) * m_serialData->m_insts.getCount());

    // Need to set up the actual instructions
    {
        const Index numInsts = m_insts.getCount();

        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            Ser::Inst& dstInst = m_serialData->m_insts[i];
            
            // Can't be any pseudo ops
            SLANG_ASSERT(!isPseudoOp(srcInst->op)); 

            dstInst.m_op = uint8_t(srcInst->op & kIROpMeta_OpMask);
            dstInst.m_payloadType = PayloadType::Empty;
            
            dstInst.m_resultTypeIndex = getInstIndex(srcInst->getFullType());

            IRConstant* irConst = as<IRConstant>(srcInst);
            if (irConst)
            {
                switch (srcInst->op)
                {
                    // Special handling for the ir const derived types
                    case kIROp_StringLit:
                    {
                        auto stringLit = static_cast<IRStringLit*>(srcInst);
                        dstInst.m_payloadType = PayloadType::String_1;
                        dstInst.m_payload.m_stringIndices[0] = getStringIndex(stringLit->getStringSlice());
                        break;
                    }
                    case kIROp_IntLit:
                    {
                        dstInst.m_payloadType = PayloadType::Int64;
                        dstInst.m_payload.m_int64 = irConst->value.intVal;
                        break;
                    }
                    case kIROp_PtrLit:
                    {
                        dstInst.m_payloadType = PayloadType::Int64;
                        dstInst.m_payload.m_int64 = (intptr_t) irConst->value.ptrVal;
                        break;
                    }
                    case kIROp_FloatLit:
                    {
                        dstInst.m_payloadType = PayloadType::Float64;
                        dstInst.m_payload.m_float64 = irConst->value.floatVal; 
                        break;
                    }
                    case kIROp_BoolLit:
                    {
                        dstInst.m_payloadType = PayloadType::UInt32;
                        dstInst.m_payload.m_uint32 = irConst->value.intVal ? 1 : 0;
                        break;
                    }
                    default:
                    {
                        SLANG_RELEASE_ASSERT(!"Unhandled constant type");
                        return SLANG_FAIL;
                    }
                }
                continue;
            }

            IRTextureTypeBase* textureBase = as<IRTextureTypeBase>(srcInst);
            if (textureBase)
            {
                dstInst.m_payloadType = PayloadType::OperandAndUInt32;
                dstInst.m_payload.m_operandAndUInt32.m_uint32 = uint32_t(srcInst->op) >> kIROpMeta_OtherShift;
                dstInst.m_payload.m_operandAndUInt32.m_operand = getInstIndex(textureBase->getElementType());
                continue;
            }

            // ModuleInst is different, in so far as it holds a pointer to IRModule, but we don't need 
            // to save that off in a special way, so can just use regular path
             
            const int numOperands = int(srcInst->operandCount);
            Ser::InstIndex* dstOperands = nullptr;

            if (numOperands <= Ser::Inst::kMaxOperands)
            {
                // Checks the compile below is valid
                SLANG_COMPILE_TIME_ASSERT(PayloadType(0) == PayloadType::Empty && PayloadType(1) == PayloadType::Operand_1 && PayloadType(2) == PayloadType::Operand_2);
                
                dstInst.m_payloadType = PayloadType(numOperands);
                dstOperands = dstInst.m_payload.m_operands;
            }
            else
            {
                dstInst.m_payloadType = PayloadType::OperandExternal;

                int operandArrayBaseIndex = int(m_serialData->m_externalOperands.getCount());
                m_serialData->m_externalOperands.setCount(operandArrayBaseIndex + numOperands);

                dstOperands = m_serialData->m_externalOperands.begin() + operandArrayBaseIndex; 

                auto& externalOperands = dstInst.m_payload.m_externalOperand;
                externalOperands.m_arrayIndex = Ser::ArrayIndex(operandArrayBaseIndex);
                externalOperands.m_size = Ser::SizeType(numOperands);
            }

            for (int j = 0; j < numOperands; ++j)
            {
                const Ser::InstIndex dstInstIndex = getInstIndex(srcInst->getOperand(j));
                dstOperands[j] = dstInstIndex;
            }
        }
    }

    // Convert strings into a string table
    {
        SerialStringTableUtil::encodeStringTable(m_stringSlicePool, serialData->m_stringTable);
    }

    // If the option to use RawSourceLocations is enabled, serialize out as is
    if (options & OptionFlag::RawSourceLocation)
    {
        const Index numInsts = m_insts.getCount();
        serialData->m_rawSourceLocs.setCount(numInsts);

        Ser::RawSourceLoc* dstLocs =  serialData->m_rawSourceLocs.begin();
        // 0 is null, just mark as no location
        dstLocs[0] = Ser::RawSourceLoc(0);
        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            dstLocs[i] = Ser::RawSourceLoc(srcInst->sourceLoc.getRaw());
        }
    }

    if (options & OptionFlag::DebugInfo)
    {
        _calcDebugInfo();
    }

    m_serialData = nullptr;
    return SLANG_OK;
}

template <typename T>
static size_t _calcChunkSize(IRSerialBinary::CompressionType compressionType, const List<T>& array)
{
    typedef IRSerialBinary Bin;

    if (array.getCount())
    {
        switch (compressionType)
        {
            case Bin::CompressionType::None:
            {
                const size_t size = sizeof(Bin::ArrayHeader) + sizeof(T) * array.getCount();
                return (size + 3) & ~size_t(3);
            }
            case Bin::CompressionType::VariableByteLite:
            {
                const size_t payloadSize = ByteEncodeUtil::calcEncodeLiteSizeUInt32((const uint32_t*)array.begin(), (array.getCount() * sizeof(T)) / sizeof(uint32_t));
                const size_t size = sizeof(Bin::CompressedArrayHeader) + payloadSize;
                return (size + 3) & ~size_t(3);
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled compression type");
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }
}

static Result _writeArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const void* data, size_t numEntries, size_t typeSize, Stream* stream)
{
    typedef IRSerialBinary Bin;

    if (numEntries == 0)
    {
        return SLANG_OK;
    }

    size_t payloadSize;

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            payloadSize = sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + typeSize * numEntries;

            Bin::ArrayHeader header;
            header.m_chunk.m_type = chunkId;
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(numEntries);

            stream->Write(&header, sizeof(header));

            stream->Write(data, typeSize * numEntries);
            break;
        }
        case Bin::CompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;

            size_t numCompressedEntries = (numEntries * typeSize) / sizeof(uint32_t);

            ByteEncodeUtil::encodeLiteUInt32((const uint32_t*)data, numCompressedEntries, compressedPayload);

            payloadSize = sizeof(Bin::CompressedArrayHeader) - sizeof(Bin::Chunk) + compressedPayload.getCount();

            Bin::CompressedArrayHeader header;
            header.m_chunk.m_type = SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId);
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(numEntries);
            header.m_numCompressedEntries = uint32_t(numCompressedEntries);

            stream->Write(&header, sizeof(header));

            stream->Write(compressedPayload.begin(), compressedPayload.getCount());
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }
    // All chunks have sizes rounded to dword size
    if (payloadSize & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - (payloadSize & 3);
        stream->Write(pad, padSize);
    }

    return SLANG_OK;
}

template <typename T>
Result _writeArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const List<T>& array, Stream* stream)
{
    return _writeArrayChunk(compressionType, chunkId, array.begin(), size_t(array.getCount()), sizeof(T), stream);
}

Result _encodeInsts(IRSerialBinary::CompressionType compressionType, const List<IRSerialData::Inst>& instsIn, List<uint8_t>& encodeArrayOut)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != Bin::CompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }
    encodeArrayOut.clear();
    
    const size_t numInsts = size_t(instsIn.getCount());
    const IRSerialData::Inst* insts = instsIn.begin();
        
    uint8_t* encodeOut = encodeArrayOut.begin();
    uint8_t* encodeEnd = encodeArrayOut.end();

    // Calculate the maximum instruction size with worst case possible encoding
    // 2 bytes hold the payload size, and the result type
    // Note that if there were some free bits, we could encode some of this stuff into bits, but if we remove payloadType, then there are no free bits
    const size_t maxInstSize = 2 + ByteEncodeUtil::kMaxLiteEncodeUInt32 + Math::Max(sizeof(insts->m_payload.m_float64), size_t(2 * ByteEncodeUtil::kMaxLiteEncodeUInt32));

    for (size_t i = 0; i < numInsts; ++i)
    {
        const auto& inst = insts[i];

        // Make sure there is space for the largest possible instruction
        if (encodeOut + maxInstSize >= encodeEnd)
        {
            const size_t offset = size_t(encodeOut - encodeArrayOut.begin());
            
            const UInt oldCapacity = encodeArrayOut.getCapacity();

            encodeArrayOut.reserve(oldCapacity + (oldCapacity >> 1) + maxInstSize);
            const UInt capacity = encodeArrayOut.getCapacity();
            encodeArrayOut.setCount(capacity);

            encodeOut = encodeArrayOut.begin() + offset;
            encodeEnd = encodeArrayOut.end();
        }

        *encodeOut++ = uint8_t(inst.m_op);
        *encodeOut++ = uint8_t(inst.m_payloadType);

        encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_resultTypeIndex, encodeOut);

        switch (inst.m_payloadType)
        {
            case PayloadType::Empty:
            {
                break;
            }
            case PayloadType::Operand_1:
            case PayloadType::String_1:
            case PayloadType::UInt32:
            {
                // 1 UInt32
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[0], encodeOut);
                break;
            }
            case PayloadType::Operand_2:
            case PayloadType::OperandAndUInt32:
            case PayloadType::OperandExternal:
            case PayloadType::String_2:
            {
                // 2 UInt32
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[0], encodeOut);
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[1], encodeOut);
                break;
            }
            case PayloadType::Float64:
            {
                memcpy(encodeOut, &inst.m_payload.m_float64, sizeof(inst.m_payload.m_float64));
                encodeOut += sizeof(inst.m_payload.m_float64);
                break;
            }
            case PayloadType::Int64:
            {
                memcpy(encodeOut, &inst.m_payload.m_int64, sizeof(inst.m_payload.m_int64));
                encodeOut += sizeof(inst.m_payload.m_int64);
                break;
            }
        }
    }

    // Fix the size 
    encodeArrayOut.setCount(UInt(encodeOut - encodeArrayOut.begin()));
    return SLANG_OK;
}

Result _writeInstArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const List<IRSerialData::Inst>& array, Stream* stream)
{
    typedef IRSerialBinary Bin;
    if (array.getCount() == 0)
    {
        return SLANG_OK;
    }

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            return _writeArrayChunk(compressionType, chunkId, array, stream);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;
            SLANG_RETURN_ON_FAIL(_encodeInsts(compressionType, array, compressedPayload));
            
            size_t payloadSize = sizeof(Bin::CompressedArrayHeader) - sizeof(Bin::Chunk) + compressedPayload.getCount();

            Bin::CompressedArrayHeader header;
            header.m_chunk.m_type = SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId);
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(array.getCount());
            header.m_numCompressedEntries = 0;          

            stream->Write(&header, sizeof(header));
            stream->Write(compressedPayload.begin(), compressedPayload.getCount());
    
            // All chunks have sizes rounded to dword size
            if (payloadSize & 3)
            {
                const uint8_t pad[4] = { 0, 0, 0, 0 };
                // Pad outs
                int padSize = 4 - (payloadSize & 3);
                stream->Write(pad, padSize);
            }
            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

static size_t _calcInstChunkSize(IRSerialBinary::CompressionType compressionType, const List<IRSerialData::Inst>& instsIn)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            return _calcChunkSize(compressionType, instsIn);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            size_t size = sizeof(Bin::CompressedArrayHeader);

            size_t numInsts = size_t(instsIn.getCount());
            size += numInsts * 2;           // op and payload

            IRSerialData::Inst* insts = instsIn.begin();

            for (size_t i = 0; i < numInsts; ++i)
            {
                const auto& inst = insts[i];

                size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_resultTypeIndex);

                switch (inst.m_payloadType)
                {
                    case PayloadType::Empty:    
                    {
                        break;
                    }
                    case PayloadType::Operand_1:
                    case PayloadType::String_1:
                    case PayloadType::UInt32:
                    {
                        // 1 UInt32
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[0]);
                        break;
                    }
                    case PayloadType::Operand_2:
                    case PayloadType::OperandAndUInt32:
                    case PayloadType::OperandExternal:
                    case PayloadType::String_2:
                    {
                        // 2 UInt32
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[0]);
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[1]);
                        break;
                    }
                    case PayloadType::Float64:
                    {
                        size += sizeof(inst.m_payload.m_float64);
                        break;
                    }
                    case PayloadType::Int64:
                    {
                        size += sizeof(inst.m_payload.m_int64);
                        break;
                    }
                }
            }

            return (size + 3) & ~size_t(3);
        }
        default: break;
    }

    SLANG_ASSERT(!"Unhandled compression type");
    return 0;
}

/* static */Result IRSerialWriter::writeStream(const IRSerialData& data, Bin::CompressionType compressionType, Stream* stream)
{
    size_t totalSize = 0;
    
    totalSize += sizeof(Bin::SlangHeader) +
        _calcInstChunkSize(compressionType, data.m_insts) +
        _calcChunkSize(compressionType, data.m_childRuns) +
        _calcChunkSize(compressionType, data.m_externalOperands) +
        _calcChunkSize(Bin::CompressionType::None, data.m_stringTable) +
        _calcChunkSize(Bin::CompressionType::None, data.m_rawSourceLocs);

    if (data.m_debugSourceInfos.getCount())
    {
        totalSize += _calcChunkSize(Bin::CompressionType::None, data.m_debugStringTable) +
            _calcChunkSize(Bin::CompressionType::None, data.m_debugLineInfos) +
            _calcChunkSize(Bin::CompressionType::None, data.m_debugAdjustedLineInfos) +
            _calcChunkSize(Bin::CompressionType::None, data.m_debugSourceInfos) +
            _calcChunkSize(compressionType, data.m_debugSourceLocRuns);
    }

    {
        Bin::Chunk riffHeader;
        riffHeader.m_type = Bin::kRiffFourCc;
        riffHeader.m_size = uint32_t(totalSize);
        
        stream->Write(&riffHeader, sizeof(riffHeader));
    }
    {
        Bin::SlangHeader slangHeader;
        slangHeader.m_chunk.m_type = Bin::kSlangFourCc;
        slangHeader.m_chunk.m_size = uint32_t(sizeof(slangHeader) - sizeof(Bin::Chunk));
        slangHeader.m_compressionType = uint32_t(Bin::CompressionType::VariableByteLite);

        stream->Write(&slangHeader, sizeof(slangHeader));
    }

    SLANG_RETURN_ON_FAIL(_writeInstArrayChunk(compressionType, Bin::kInstFourCc, data.m_insts, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kChildRunFourCc, data.m_childRuns, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kExternalOperandsFourCc, data.m_externalOperands, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(Bin::CompressionType::None, Bin::kStringFourCc, data.m_stringTable, stream));

    SLANG_RETURN_ON_FAIL(_writeArrayChunk(Bin::CompressionType::None, Bin::kUInt32SourceLocFourCc, data.m_rawSourceLocs, stream));

    if (data.m_debugSourceInfos.getCount())
    {
        _writeArrayChunk(Bin::CompressionType::None, Bin::kDebugStringFourCc, data.m_debugStringTable, stream);
        _writeArrayChunk(Bin::CompressionType::None, Bin::kDebugLineInfoFourCc, data.m_debugLineInfos, stream);
        _writeArrayChunk(Bin::CompressionType::None, Bin::kDebugAdjustedLineInfoFourCc, data.m_debugAdjustedLineInfos, stream);
        _writeArrayChunk(Bin::CompressionType::None, Bin::kDebugSourceInfoFourCc, data.m_debugSourceInfos, stream);
        _writeArrayChunk(compressionType, Bin::kDebugSourceLocRunFourCc, data.m_debugSourceLocRuns, stream);
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class ListResizer
{
    public:
    virtual void* setSize(size_t newSize) = 0;
    SLANG_FORCE_INLINE size_t getTypeSize() const { return m_typeSize; }
    ListResizer(size_t typeSize):m_typeSize(typeSize) {}

    protected:
    size_t m_typeSize;
};

template <typename T>
class ListResizerForType: public ListResizer
{
    public:
    typedef ListResizer Parent;

    SLANG_FORCE_INLINE ListResizerForType(List<T>& list):
        Parent(sizeof(T)), 
        m_list(list) 
    {}
    
    virtual void* setSize(size_t newSize) SLANG_OVERRIDE
    {
        m_list.setCount(UInt(newSize));
        return (void*)m_list.begin();
    }

    protected:
    List<T>& m_list;
};

static Result _readArrayChunk(IRSerialBinary::CompressionType compressionType, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, ListResizer& listOut)
{
    typedef IRSerialBinary Bin;

    const size_t typeSize = listOut.getTypeSize();

    switch (compressionType)
    {
        case Bin::CompressionType::VariableByteLite:
        {
            // We have a compressed header
            Bin::CompressedArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);

            void* data = listOut.setSize(header.m_numEntries);
            
            // Need to read all the compressed data... 
            size_t payloadSize = header.m_chunk.m_size - (sizeof(header) - sizeof(Bin::Chunk));

            List<uint8_t> compressedPayload;
            compressedPayload.setCount(payloadSize);

            stream->Read(compressedPayload.begin(), payloadSize);
            *numReadInOut += payloadSize;
        
            SLANG_ASSERT(header.m_numCompressedEntries == uint32_t((header.m_numEntries * typeSize) / sizeof(uint32_t)));

            // Decode..
            ByteEncodeUtil::decodeLiteUInt32(compressedPayload.begin(), header.m_numCompressedEntries, (uint32_t*)data);
            break;
        }
        case Bin::CompressionType::None:
        {
            // Read uncompressed
            Bin::ArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);

            const size_t payloadSize = header.m_numEntries * typeSize;

            void* data = listOut.setSize(header.m_numEntries);

            stream->Read(data, payloadSize);
            *numReadInOut += payloadSize;
            break;
        }
    }

    // All chunks have sizes rounded to dword size
    if (*numReadInOut & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - int(*numReadInOut & 3);
        stream->Seek(SeekOrigin::Current, padSize);

        *numReadInOut += padSize;
    }

    return SLANG_OK;
}

template <typename T>
Result _readArrayChunk(const IRSerialBinary::SlangHeader& header, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::CompressionType compressionType = Bin::CompressionType::None;

    if (chunk.m_type == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk.m_type))
    {
        // If it has compression, use the compression type set in the header
        compressionType = Bin::CompressionType(header.m_compressionType);
    }
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(compressionType, chunk, stream, numReadInOut, resizer);
}  

template <typename T>
Result _readArrayUncompressedChunk(const IRSerialBinary::SlangHeader& header, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;
    SLANG_UNUSED(header);
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(Bin::CompressionType::None, chunk, stream, numReadInOut, resizer);
}

static Result _decodeInsts(IRSerialBinary::CompressionType compressionType, const List<uint8_t>& encodeIn, List<IRSerialData::Inst>& instsOut)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != Bin::CompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }

    const size_t numInsts = size_t(instsOut.getCount());
    IRSerialData::Inst* insts = instsOut.begin();

    const uint8_t* encodeCur = encodeIn.begin();

    for (size_t i = 0; i < numInsts; ++i)
    {
        auto& inst = insts[i];

        inst.m_op = *encodeCur++;
        const PayloadType payloadType = PayloadType(*encodeCur++);
        inst.m_payloadType = payloadType;
        
        // Read the result value
        encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, (uint32_t*)&inst.m_resultTypeIndex);

        switch (inst.m_payloadType)
        {
            case PayloadType::Empty:
            {
                break;
            }
            case PayloadType::Operand_1:
            case PayloadType::String_1:
            case PayloadType::UInt32:
            {
                // 1 UInt32
                encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, (uint32_t*)&inst.m_payload.m_operands[0]);
                break;
            }
            case PayloadType::Operand_2:
            case PayloadType::OperandAndUInt32:
            case PayloadType::OperandExternal:
            case PayloadType::String_2:
            {
                // 2 UInt32
                encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, 2, (uint32_t*)&inst.m_payload.m_operands[0]);
                break;
            }
            case PayloadType::Float64:
            {
                memcpy(&inst.m_payload.m_float64, encodeCur, sizeof(inst.m_payload.m_float64));
                encodeCur += sizeof(inst.m_payload.m_float64);
                break;
            }
            case PayloadType::Int64:
            {
                memcpy(&inst.m_payload.m_int64, encodeCur, sizeof(inst.m_payload.m_int64));
                encodeCur += sizeof(inst.m_payload.m_int64);
                break;
            }
        }
    }

    return SLANG_OK;
}

Result _readInstArrayChunk(const IRSerialBinary::SlangHeader& slangHeader, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<IRSerialData::Inst>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::CompressionType compressionType = Bin::CompressionType::None;
    if (chunk.m_type == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk.m_type))
    {
        compressionType = Bin::CompressionType(slangHeader.m_compressionType);
    }

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            ListResizerForType<IRSerialData::Inst> resizer(arrayOut);
            return _readArrayChunk(compressionType, chunk, stream, numReadInOut, resizer);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            // We have a compressed header
            Bin::CompressedArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);
            
            // Need to read all the compressed data... 
            size_t payloadSize = header.m_chunk.m_size - (sizeof(header) - sizeof(Bin::Chunk));

            List<uint8_t> compressedPayload;
            compressedPayload.setCount(payloadSize);

            stream->Read(compressedPayload.begin(), payloadSize);
            *numReadInOut += payloadSize;

            arrayOut.setCount(header.m_numEntries);

            SLANG_RETURN_ON_FAIL(_decodeInsts(compressionType, compressedPayload, arrayOut));
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }

    // All chunks have sizes rounded to dword size
    if (*numReadInOut & 3)
    {
        // Pad outs
        int padSize = 4 - int(*numReadInOut & 3);
        stream->Seek(SeekOrigin::Current, padSize);
        *numReadInOut += padSize;
    }

    return SLANG_OK;
}

int64_t _calcChunkTotalSize(const IRSerialBinary::Chunk& chunk)
{
    int64_t size = chunk.m_size + sizeof(IRSerialBinary::Chunk);
    return (size + 3) & ~int64_t(3);
}

/* static */Result IRSerialReader::_skip(const IRSerialBinary::Chunk& chunk, Stream* stream, int64_t* remainingBytesInOut)
{
    typedef IRSerialBinary Bin;
    int64_t chunkSize = _calcChunkTotalSize(chunk);
    if (remainingBytesInOut)
    {
        *remainingBytesInOut -= chunkSize;
    }

    // Skip the payload (we don't need to skip the Chunk because that was already read
    stream->Seek(SeekOrigin::Current, chunkSize - sizeof(IRSerialBinary::Chunk));
    return SLANG_OK;
}

/* static */Result IRSerialReader::readStream(Stream* stream, IRSerialData* dataOut)
{
    typedef IRSerialBinary Bin;

    dataOut->clear();

    int64_t remainingBytes = 0;
    {
        Bin::Chunk header;
        stream->Read(&header, sizeof(header));
        if (header.m_type != Bin::kRiffFourCc)
        {
            return SLANG_FAIL;
        }

        remainingBytes = header.m_size;
    }
    
    // Header
    // Chunk will not be kSlangFourCC if not read yet
    Bin::SlangHeader slangHeader;
    memset(&slangHeader, 0, sizeof(slangHeader));

    while (remainingBytes > 0)
    {
        Bin::Chunk chunk;

        stream->Read(&chunk, sizeof(chunk));

        size_t bytesRead = sizeof(chunk);

        switch (chunk.m_type)
        {
            case Bin::kSlangFourCc:
            {
                // Slang header
                slangHeader.m_chunk = chunk;

                // NOTE! Really we should only read what we know the size to be...
                // and skip if it's larger

                stream->Read(&slangHeader.m_chunk + 1, sizeof(slangHeader) - sizeof(chunk));

                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kInstFourCc):
            case Bin::kInstFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readInstArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_insts));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;    
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kChildRunFourCc):
            case Bin::kChildRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_childRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kExternalOperandsFourCc):
            case Bin::kExternalOperandsFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_externalOperands));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_stringTable));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kUInt32SourceLocFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_rawSourceLocs));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kDebugStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_debugStringTable));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kDebugLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_debugLineInfos));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kDebugAdjustedLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_debugAdjustedLineInfos));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kDebugSourceInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_debugSourceInfos));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kDebugSourceLocRunFourCc):
            case Bin::kDebugSourceLocRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_debugSourceLocRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            } 

            default:
            {
                SLANG_RETURN_ON_FAIL(_skip(chunk, stream, &remainingBytes));
                break;
            }
        }
    }

    return SLANG_OK;
}

static SourceRange _toSourceRange(const IRSerialData::DebugSourceInfo& info)
{
    SourceRange range;
    range.begin = SourceLoc::fromRaw(info.m_startSourceLoc);
    range.end = SourceLoc::fromRaw(info.m_endSourceLoc);
    return range;
}

static int _findIndex(const List<IRSerialData::DebugSourceInfo>& infos, SourceLoc sourceLoc)
{
    const int numInfos = int(infos.getCount());
    for (int i = 0; i < numInfos; ++i)
    {
        if (_toSourceRange(infos[i]).contains(sourceLoc))
        {
            return i;
        }
    }

    return -1;
}

static int _calcFixSourceLoc(const IRSerialData::DebugSourceInfo& info, SourceView* sourceView, SourceRange& rangeOut)
{
    rangeOut = _toSourceRange(info);
    return int(sourceView->getRange().begin.getRaw()) - int(info.m_startSourceLoc);
}

/* static */Result IRSerialReader::read(const IRSerialData& data, Session* session, SourceManager* sourceManager, RefPtr<IRModule>& moduleOut)
{
    typedef Ser::Inst::PayloadType PayloadType;

    m_serialData = &data;
 
    auto module = new IRModule();
    moduleOut = module;
    m_module = module;

    module->session = session;

    // Set up the string rep cache
    m_stringRepresentationCache.init(&data.m_stringTable, session->getNamePool(), module->getObjectScopeManager());
    
    // Add all the instructions

    List<IRInst*> insts;

    const Index numInsts = data.m_insts.getCount();

    SLANG_ASSERT(numInsts > 0);

    insts.setCount(numInsts);
    insts[0] = nullptr;

    // 0 holds null
    // 1 holds the IRModuleInst
    {
        // Check that insts[1] is the module inst
        const Ser::Inst& srcInst = data.m_insts[1];
        SLANG_RELEASE_ASSERT(srcInst.m_op == kIROp_Module);
        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);

        // Create the module inst
        auto moduleInst = static_cast<IRModuleInst*>(createEmptyInstWithSize(module, kIROp_Module, sizeof(IRModuleInst)));
        module->moduleInst = moduleInst;
        moduleInst->module = module;

        // Set the IRModuleInst
        insts[1] = moduleInst; 
    }

    for (Index i = 2; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];

        const IROp op((IROp)srcInst.m_op);

        if (isConstant(op))
        {
            // Handling of constants

            // Calculate the minimum object size (ie not including the payload of value)    
            const size_t prefixSize = SLANG_OFFSET_OF(IRConstant, value);

            IRConstant* irConst = nullptr;
            switch (op)
            {                    
                case kIROp_BoolLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);
                    irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, prefixSize + sizeof(IRIntegerValue)));
                    irConst->value.intVal = srcInst.m_payload.m_uint32 != 0;
                    break;
                }
                case kIROp_IntLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Int64);
                    irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, prefixSize + sizeof(IRIntegerValue)));
                    irConst->value.intVal = srcInst.m_payload.m_int64; 
                    break;
                }
                case kIROp_PtrLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Int64);
                    irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, prefixSize + sizeof(void*)));
                    irConst->value.ptrVal = (void*) (intptr_t) srcInst.m_payload.m_int64; 
                    break;
                }
                case kIROp_FloatLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Float64);
                    irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op,  prefixSize + sizeof(IRFloatingPointValue)));
                    irConst->value.floatVal = srcInst.m_payload.m_float64;
                    break;
                }
                case kIROp_StringLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);

                    const UnownedStringSlice slice = m_stringRepresentationCache.getStringSlice(StringHandle(srcInst.m_payload.m_stringIndices[0]));
                        
                    const size_t sliceSize = slice.size();
                    const size_t instSize = prefixSize + SLANG_OFFSET_OF(IRConstant::StringValue, chars) + sliceSize;

                    irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, instSize));

                    IRConstant::StringValue& dstString = irConst->value.stringVal;

                    dstString.numChars = uint32_t(sliceSize);
                    // Turn into pointer to avoid warning of array overrun
                    char* dstChars = dstString.chars;
                    // Copy the chars
                    memcpy(dstChars, slice.begin(), sliceSize);
                    break;
                }
                default:
                {
                    SLANG_ASSERT(!"Unknown constant type");
                    return SLANG_FAIL;
                }
            }

            insts[i] = irConst;
        }
        else if (isTextureTypeBase(op))
        {
            IRTextureTypeBase* inst = static_cast<IRTextureTypeBase*>(createEmptyInst(module, op, 1));
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::OperandAndUInt32);

            // Reintroduce the texture type bits into the the
            const uint32_t other = srcInst.m_payload.m_operandAndUInt32.m_uint32;
            inst->op = IROp(uint32_t(inst->op) | (other << kIROpMeta_OtherShift));

            insts[i] = inst;
        }
        else
        {
            int numOperands = srcInst.getNumOperands();
            insts[i] = createEmptyInst(module, op, numOperands);
        }
    }

    // Patch up the operands
    for (Index i = 1; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];
        const IROp op((IROp)srcInst.m_op);

        IRInst* dstInst = insts[i];

        // Set the result type
        if (srcInst.m_resultTypeIndex != Ser::InstIndex(0))
        {
            IRInst* resultInst = insts[int(srcInst.m_resultTypeIndex)];
            // NOTE! Counter intuitively the IRType* paramter may not be IRType* derived for example 
            // IRGlobalGenericParam is valid, but isn't IRType* derived

            //SLANG_RELEASE_ASSERT(as<IRType>(resultInst));
            dstInst->setFullType(static_cast<IRType*>(resultInst));
        }
       
        //if (!isParentDerived(op))
        {
            const Ser::InstIndex* srcOperandIndices;
            const int numOperands = data.getOperands(srcInst, &srcOperandIndices);
                         
            for (int j = 0; j < numOperands; j++)
            {
                dstInst->setOperand(j, insts[int(srcOperandIndices[j])]);
            }
        }
    }
    
    // Patch up the children
    {
        const Index numChildRuns = data.m_childRuns.getCount();
        for (Index i = 0; i < numChildRuns; i++)
        {
            const auto& run = data.m_childRuns[i];

            IRInst* inst = insts[int(run.m_parentIndex)];

            for (int j = 0; j < int(run.m_numChildren); ++j)
            {
                IRInst* child = insts[j + int(run.m_startInstIndex)];
                SLANG_ASSERT(child->parent == nullptr);
                child->insertAtEnd(inst);
            }
        }
    }

    // Re-add source locations, if they are defined
    if (m_serialData->m_rawSourceLocs.getCount() == numInsts)
    {
        const Ser::RawSourceLoc* srcLocs = m_serialData->m_rawSourceLocs.begin();
        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* dstInst = insts[i];
            
            dstInst->sourceLoc.setRaw(Slang::SourceLoc::RawValue(srcLocs[i]));
        }
    }

    if (sourceManager && m_serialData->m_debugSourceInfos.getCount())
    {
        List<UnownedStringSlice> debugStringSlices;
        SerialStringTableUtil::decodeStringTable(m_serialData->m_debugStringTable, debugStringSlices);

        // All of the strings are placed in the manager (and its StringSlicePool) where the SourceView and SourceFile are constructed from
        List<StringSlicePool::Handle> stringMap;
        SerialStringTableUtil::calcStringSlicePoolMap(debugStringSlices, sourceManager->getStringSlicePool(), stringMap);

        const List<IRSerialData::DebugSourceInfo>& sourceInfos = m_serialData->m_debugSourceInfos;

        // Construct the source files
        Index numSourceFiles = sourceInfos.getCount();

        // These hold the views (and SourceFile as there is only one SourceFile per view) in the same order as the sourceInfos
        List<SourceView*> sourceViews;
        sourceViews.setCount(numSourceFiles);

        for (Index i = 0; i < numSourceFiles; ++i)
        {
            const IRSerialData::DebugSourceInfo& srcSourceInfo = sourceInfos[i];

            PathInfo pathInfo;
            pathInfo.type = PathInfo::Type::FoundPath;
            pathInfo.foundPath = debugStringSlices[UInt(srcSourceInfo.m_pathIndex)];

            SourceFile* sourceFile = sourceManager->createSourceFileWithSize(pathInfo, srcSourceInfo.m_endSourceLoc - srcSourceInfo.m_startSourceLoc);
            SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr);

            // We need to accumulate all line numbers, for this source file, both adjusted and unadjusted
            List<IRSerialData::DebugLineInfo> lineInfos;
            // Add the adjusted lines
            {
                lineInfos.setCount(srcSourceInfo.m_numAdjustedLineInfos);
                const IRSerialData::DebugAdjustedLineInfo* srcAdjustedLineInfos = m_serialData->m_debugAdjustedLineInfos.getBuffer() + srcSourceInfo.m_adjustedLineInfosStartIndex;
                const int numAdjustedLines = int(srcSourceInfo.m_numAdjustedLineInfos);
                for (int j = 0; j < numAdjustedLines; ++j)
                {
                    lineInfos[j] = srcAdjustedLineInfos[j].m_lineInfo;
                }
            }
            // Add regular lines
            lineInfos.addRange(m_serialData->m_debugLineInfos.getBuffer() + srcSourceInfo.m_lineInfosStartIndex, srcSourceInfo.m_numLineInfos);
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
                const uint32_t offset = uint32_t(srcSourceInfo.m_endSourceLoc - srcSourceInfo.m_startSourceLoc);
                for (; lineIndex < numLines; ++lineIndex)
                {
                    lineBreakOffsets[lineIndex] = offset;
                }
            }

            sourceFile->setLineBreakOffsets(lineBreakOffsets.getBuffer(), lineBreakOffsets.getCount());

            if (srcSourceInfo.m_numAdjustedLineInfos)
            {
                List<IRSerialData::DebugAdjustedLineInfo> adjustedLineInfos;

                int numEntries = int(srcSourceInfo.m_numAdjustedLineInfos);

                adjustedLineInfos.addRange(m_serialData->m_debugAdjustedLineInfos.getBuffer() + srcSourceInfo.m_adjustedLineInfosStartIndex, numEntries);
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

            sourceViews[i] = sourceView;
        }

        // We now need to apply the runs
        {
            List<IRSerialData::SourceLocRun> sourceRuns(m_serialData->m_debugSourceLocRuns);
            // They are now in source location order
            sourceRuns.sort();

            // Just guess initially 0 for the source file that contains the initial run
            SourceRange range;
            int fixSourceLoc = _calcFixSourceLoc(sourceInfos[0], sourceViews[0], range);

            const Index numRuns = sourceRuns.getCount();
            for (Index i = 0; i < numRuns; ++i)
            {
                const auto& run = sourceRuns[i];
                const SourceLoc srcSourceLoc = SourceLoc::fromRaw(run.m_sourceLoc);

                if (!range.contains(srcSourceLoc))
                {
                    int index = _findIndex(sourceInfos, srcSourceLoc);
                    if (index < 0)
                    {
                        // Didn't find the match
                        continue;
                    }
                    fixSourceLoc = _calcFixSourceLoc(sourceInfos[index], sourceViews[index], range);
                    SLANG_ASSERT(range.contains(srcSourceLoc));
                }

                // Work out the fixed source location
                SourceLoc sourceLoc = SourceLoc::fromRaw(int(run.m_sourceLoc) + fixSourceLoc); 

                SLANG_ASSERT(Index(uint32_t(run.m_startInstIndex) + run.m_numInst) <= insts.getCount());
                IRInst** dstInsts = insts.getBuffer() + int(run.m_startInstIndex);

                const int runSize = int(run.m_numInst);
                for (int j = 0; j < runSize; ++j)
                {
                    dstInsts[j]->sourceLoc = sourceLoc;
                }
            }
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void IRSerialUtil::calcInstructionList(IRModule* module, List<IRInst*>& instsOut)
{
    // We reserve 0 for null
    instsOut.setCount(1);
    instsOut[0] = nullptr;

    // Stack for parentInst
    List<IRInst*> parentInstStack;

    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.add(moduleInst);

    // Add to list
    instsOut.add(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.getCount())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRInst* parentInst = parentInstStack.getLast();
        parentInstStack.removeLast();

        IRInstListBase childrenList = parentInst->getDecorationsAndChildren();
        for (IRInst* child : childrenList)
        {
            instsOut.add(child);
            parentInstStack.add(child);
        }
    }
}

/* static */SlangResult IRSerialUtil::verifySerialize(IRModule* module, Session* session, SourceManager* sourceManager, IRSerialBinary::CompressionType compressionType, IRSerialWriter::OptionFlags optionFlags)
{
    // Verify if we can stream out with debug information
        
    List<IRInst*> originalInsts;
    calcInstructionList(module, originalInsts);
    
    IRSerialData serialData;
    {
        // Write IR out to serialData - copying over SourceLoc information directly
        IRSerialWriter writer;
        SLANG_RETURN_ON_FAIL(writer.write(module, sourceManager, optionFlags, &serialData));
    }

    // Write the data out to stream
    MemoryStream memoryStream(FileAccess::ReadWrite);
    SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, compressionType, &memoryStream));

    // Reset stream
    memoryStream.Seek(SeekOrigin::Start, 0);

    IRSerialData readData;

    SLANG_RETURN_ON_FAIL(IRSerialReader::readStream(&memoryStream, &readData));

    // Check the stream read data is the same
    if (readData != serialData)
    {
        SLANG_ASSERT(!"Streamed in data doesn't match");
        return SLANG_FAIL;
    }

    RefPtr<IRModule> irReadModule;

    SourceManager workSourceManager;
    workSourceManager.initialize(sourceManager, nullptr);

    {
        IRSerialReader reader;
        SLANG_RETURN_ON_FAIL(reader.read(serialData, session, &workSourceManager, irReadModule));
    }

    List<IRInst*> readInsts;
    calcInstructionList(irReadModule, readInsts);

    if (readInsts.getCount() != originalInsts.getCount())
    {
        SLANG_ASSERT(!"Instruction counts don't match");
        return SLANG_FAIL;
    }

    if (optionFlags & IRSerialWriter::OptionFlag::RawSourceLocation)
    {
        SLANG_ASSERT(readInsts[0] == originalInsts[0]);
        // All the source locs should be identical
        for (Index i = 1; i < readInsts.getCount(); ++i)
        {
            IRInst* origInst = originalInsts[i];
            IRInst* readInst = readInsts[i];

            if (origInst->sourceLoc.getRaw() != readInst->sourceLoc.getRaw())
            {
                SLANG_ASSERT(!"Source locs don't match");
                return SLANG_FAIL;
            }
        }
    }
    else if (optionFlags & IRSerialWriter::OptionFlag::DebugInfo)
    {
        // They should be on the same line nos
        for (Index i = 1; i < readInsts.getCount(); ++i)
        {
            IRInst* origInst = originalInsts[i];
            IRInst* readInst = readInsts[i];

            if (origInst->sourceLoc.getRaw() == readInst->sourceLoc.getRaw())
            {
                continue;
            }

            // Work out the
            SourceView* origSourceView = sourceManager->findSourceView(origInst->sourceLoc);
            SourceView* readSourceView = workSourceManager.findSourceView(readInst->sourceLoc);

            // if both are null we are done
            if (origSourceView == nullptr && origSourceView == readSourceView)
            {
                continue;
            }
            SLANG_ASSERT(origSourceView && readSourceView);

            {
                auto origInfo = origSourceView->getHumaneLoc(origInst->sourceLoc, SourceLocType::Actual);
                auto readInfo = readSourceView->getHumaneLoc(readInst->sourceLoc, SourceLocType::Actual);

                if (!(origInfo.line == readInfo.line && origInfo.column == readInfo.column && origInfo.pathInfo.foundPath == readInfo.pathInfo.foundPath))
                {
                    SLANG_ASSERT(!"Debug data didn't match");
                    return SLANG_FAIL;
                }
            }

            // We may have adjusted line numbers -> but they may not match, because we only reconstruct one view
            // So for now disable this test

            if (false)
            {
                auto origInfo = origSourceView->getHumaneLoc(origInst->sourceLoc, SourceLocType::Nominal);
                auto readInfo = readSourceView->getHumaneLoc(readInst->sourceLoc, SourceLocType::Nominal);

                if (!(origInfo.line == readInfo.line && origInfo.column == readInfo.column && origInfo.pathInfo.foundPath == readInfo.pathInfo.foundPath))
                {
                    SLANG_ASSERT(!"Debug data didn't match");
                    return SLANG_FAIL;
                }
            }
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#if 0

Result serializeModule(IRModule* module, SourceManager* sourceManager, Stream* stream)
{
    IRSerialWriter serializer;
    IRSerialData serialData;

    SLANG_RETURN_ON_FAIL(serializer.write(module, sourceManager, IRSerialWriter::OptionFlag::RawSourceLocation, &serialData));

    if (stream)
    {
        SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, IRSerialBinary::CompressionType::VariableByteLite, stream));
    }

    return SLANG_OK;
}

Result readModule(Session* session, Stream* stream, RefPtr<IRModule>& moduleOut)
{
    IRSerialData serialData;
    IRSerialReader::readStream(stream, &serialData);

    IRSerialReader reader;
    return reader.read(serialData, session, moduleOut);
}

#endif

} // namespace Slang
