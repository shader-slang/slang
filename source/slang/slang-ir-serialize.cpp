// slang-ir-serialize.cpp
#include "slang-ir-serialize.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "slang-ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

static bool _isTextureTypeBase(IROp opIn)
{
    const int op = (kIROpMeta_OpMask & opIn);
    return op >= kIROp_FirstTextureTypeBase && op <= kIROp_LastTextureTypeBase;
}

static bool _isConstant(IROp opIn)
{
    const int op = (kIROpMeta_OpMask & opIn);
    return op >= kIROp_FirstConstant && op <= kIROp_LastConstant;
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

            const auto& pool = sourceView->getSourceManager()->getStringSlicePool();
            SLANG_ASSERT(pool.getStyle() == StringSlicePool::Style::Default);

            if (!pool.isDefaultHandle(entry.m_pathHandle))
            {
                UnownedStringSlice slice = pool.getSlice(entry.m_pathHandle);
                SLANG_ASSERT(slice.getLength() > 0);
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

static Result _writeArrayChunk(IRSerialCompressionType compressionType, FourCC chunkId, const void* data, size_t numEntries, size_t typeSize, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    typedef IRSerialBinary Bin;
    if (numEntries == 0)
    {
        return SLANG_OK;
    }

    // Make compressed fourCC
    chunkId = (compressionType != IRSerialCompressionType::None) ? SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId) : chunkId;

    ScopeChunk scope(container, Chunk::Kind::Data, chunkId);

    switch (compressionType)
    {
        case IRSerialCompressionType::None:
        {
            Bin::ArrayHeader header;
            header.numEntries = uint32_t(numEntries);

            container->write(&header, sizeof(header));
            container->write(data, typeSize * numEntries);
            break;
        }
        case IRSerialCompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;

            size_t numCompressedEntries = (numEntries * typeSize) / sizeof(uint32_t);
            ByteEncodeUtil::encodeLiteUInt32((const uint32_t*)data, numCompressedEntries, compressedPayload);

            Bin::CompressedArrayHeader header;
            header.numEntries = uint32_t(numEntries);
            header.numCompressedEntries = uint32_t(numCompressedEntries);

            container->write(&header, sizeof(header));

            const size_t compressedSize = compressedPayload.getCount();
            container->moveOwned(container->addData(), compressedPayload.detachBuffer(), compressedSize);
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

template <typename T>
Result _writeArrayChunk(IRSerialCompressionType compressionType, FourCC chunkId, const List<T>& array, RiffContainer* container)
{
    return _writeArrayChunk(compressionType, chunkId, array.begin(), size_t(array.getCount()), sizeof(T), container);
}

Result _encodeInsts(IRSerialCompressionType compressionType, const List<IRSerialData::Inst>& instsIn, List<uint8_t>& encodeArrayOut)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != IRSerialCompressionType::VariableByteLite)
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

Result _writeInstArrayChunk(IRSerialCompressionType compressionType, FourCC chunkId, const List<IRSerialData::Inst>& array, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    typedef IRSerialBinary Bin;
    if (array.getCount() == 0)
    {
        return SLANG_OK;
    }

    switch (compressionType)
    {
        case IRSerialCompressionType::None:
        {
            return _writeArrayChunk(compressionType, chunkId, array, container);
        }
        case IRSerialCompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;
            SLANG_RETURN_ON_FAIL(_encodeInsts(compressionType, array, compressedPayload));

            ScopeChunk scope(container, Chunk::Kind::Data, SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId));

            Bin::CompressedArrayHeader header;
            header.numEntries = uint32_t(array.getCount());
            header.numCompressedEntries = 0;          

            container->write(&header, sizeof(header));

            const size_t compressedPayloadSize = compressedPayload.getCount();
            container->moveOwned(container->addData(), compressedPayload.detachBuffer(), compressedPayloadSize);

            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

/* static */Result IRSerialWriter::writeContainer(const IRSerialData& data, IRSerialCompressionType compressionType, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    ScopeChunk scopeModule(container, Chunk::Kind::List, Bin::kSlangModuleFourCc);

    // Write the header
    {
        Bin::ModuleHeader moduleHeader;
        moduleHeader.compressionType = uint32_t(IRSerialCompressionType::VariableByteLite);

        ScopeChunk scopeHeader(container, Chunk::Kind::Data, Bin::kSlangModuleHeaderFourCc);
        container->write(&moduleHeader, sizeof(moduleHeader));
    }

    SLANG_RETURN_ON_FAIL(_writeInstArrayChunk(compressionType, Bin::kInstFourCc, data.m_insts, container));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kChildRunFourCc, data.m_childRuns, container));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kExternalOperandsFourCc, data.m_externalOperands, container));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(IRSerialCompressionType::None, Bin::kStringFourCc, data.m_stringTable, container));

    SLANG_RETURN_ON_FAIL(_writeArrayChunk(IRSerialCompressionType::None, Bin::kUInt32SourceLocFourCc, data.m_rawSourceLocs, container));

    if (data.m_debugSourceInfos.getCount())
    {
        _writeArrayChunk(IRSerialCompressionType::None, Bin::kDebugStringFourCc, data.m_debugStringTable, container);
        _writeArrayChunk(IRSerialCompressionType::None, Bin::kDebugLineInfoFourCc, data.m_debugLineInfos, container);
        _writeArrayChunk(IRSerialCompressionType::None, Bin::kDebugAdjustedLineInfoFourCc, data.m_debugAdjustedLineInfos, container);
        _writeArrayChunk(IRSerialCompressionType::None, Bin::kDebugSourceInfoFourCc, data.m_debugSourceInfos, container);
        _writeArrayChunk(compressionType, Bin::kDebugSourceLocRunFourCc, data.m_debugSourceLocRuns, container);
    }

    return SLANG_OK;
}

/* static */Result IRSerialWriter::writeStream(const IRSerialData& data, IRSerialCompressionType compressionType, Stream* stream)
{
    RiffContainer container;
    SLANG_RETURN_ON_FAIL(writeContainer(data, compressionType, &container));
    return RiffUtil::write(container.getRoot(), true, stream);
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

static Result _readArrayChunk(IRSerialCompressionType compressionType, RiffContainer::DataChunk* dataChunk, ListResizer& listOut)
{
    typedef IRSerialBinary Bin;

    RiffReadHelper read = dataChunk->asReadHelper();
    const size_t typeSize = listOut.getTypeSize();

    switch (compressionType)
    {
        case IRSerialCompressionType::VariableByteLite:
        {
            Bin::CompressedArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));

            void* dst = listOut.setSize(header.numEntries);
            SLANG_ASSERT(header.numCompressedEntries == uint32_t((header.numEntries * typeSize) / sizeof(uint32_t)));

            // Decode..
            ByteEncodeUtil::decodeLiteUInt32(read.getData(), header.numCompressedEntries, (uint32_t*)dst);
            break;
        }
        case IRSerialCompressionType::None:
        {
            // Read uncompressed
            Bin::ArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));
            const size_t payloadSize = header.numEntries * typeSize;
            SLANG_ASSERT(payloadSize == read.getRemainingSize());
            void* dst = listOut.setSize(header.numEntries);
            ::memcpy(dst, read.getData(), payloadSize);
            break;
        }
    }
    return SLANG_OK;
}

template <typename T>
static Result _readArrayChunk(const IRSerialBinary::ModuleHeader* header, RiffContainer::DataChunk* dataChunk, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::CompressionType compressionType = Bin::CompressionType::None;

    if (dataChunk->m_fourCC == SLANG_MAKE_COMPRESSED_FOUR_CC(dataChunk->m_fourCC))
    {
        // If it has compression, use the compression type set in the header
        compressionType = Bin::CompressionType(header->compressionType);
    }
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(compressionType, dataChunk, resizer);
}  

template <typename T>
static Result _readArrayUncompressedChunk(const IRSerialBinary::ModuleHeader* header, RiffContainer::DataChunk* chunk, List<T>& arrayOut)
{
    SLANG_UNUSED(header);
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(IRSerialBinary::CompressionType::None, chunk, resizer);
}

static Result _decodeInsts(IRSerialCompressionType compressionType, const uint8_t* encodeCur, size_t encodeInSize, List<IRSerialData::Inst>& instsOut)
{
    const uint8_t* encodeEnd = encodeCur + encodeInSize;

    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != IRSerialCompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }

    const size_t numInsts = size_t(instsOut.getCount());
    IRSerialData::Inst* insts = instsOut.begin();

    for (size_t i = 0; i < numInsts; ++i)
    {
        if (encodeCur >= encodeEnd)
        {
            SLANG_ASSERT(!"Invalid decode");
            return SLANG_FAIL;
        }

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

static Result _readInstArrayChunk(const IRSerialBinary::ModuleHeader* moduleHeader, RiffContainer::DataChunk* chunk, List<IRSerialData::Inst>& arrayOut)
{
    typedef IRSerialBinary Bin;

    IRSerialCompressionType compressionType = IRSerialCompressionType::None;
    if (chunk->m_fourCC == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk->m_fourCC))
    {
        compressionType = IRSerialCompressionType(moduleHeader->compressionType);
    }

    switch (compressionType)
    {
        case IRSerialCompressionType::None:
        {
            ListResizerForType<IRSerialData::Inst> resizer(arrayOut);
            return _readArrayChunk(compressionType, chunk, resizer);
        }
        case IRSerialCompressionType::VariableByteLite:
        {
            RiffReadHelper read = chunk->asReadHelper();

            Bin::CompressedArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));

            arrayOut.setCount(header.numEntries);

            SLANG_RETURN_ON_FAIL(_decodeInsts(compressionType, read.getData(), read.getRemainingSize(), arrayOut));
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

/* static */Result IRSerialReader::readContainer(RiffContainer::ListChunk* module, IRSerialData* outData)
{
    typedef IRSerialBinary Bin;

    outData->clear();

    Bin::ModuleHeader* header = module->findContainedData<Bin::ModuleHeader>(Bin::kSlangModuleHeaderFourCc);
    if (!header)
    {
        return SLANG_FAIL;
    }

    for (RiffContainer::Chunk* chunk = module->m_containedChunks; chunk; chunk = chunk->m_next)
    {
        RiffContainer::DataChunk* dataChunk = as<RiffContainer::DataChunk>(chunk);
        if (!dataChunk)
        {
            continue;
        }
        
        switch (dataChunk->m_fourCC)
        {
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kInstFourCc):
            case Bin::kInstFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readInstArrayChunk(header, dataChunk, outData->m_insts));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kChildRunFourCc):
            case Bin::kChildRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(header, dataChunk, outData->m_childRuns));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kExternalOperandsFourCc):
            case Bin::kExternalOperandsFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(header, dataChunk, outData->m_externalOperands));
                break;
            }
            case Bin::kStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(header, dataChunk, outData->m_stringTable));
                break;
            }
            case Bin::kUInt32SourceLocFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(header, dataChunk, outData->m_rawSourceLocs));
                break;
            }
            case Bin::kDebugStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(header, dataChunk, outData->m_debugStringTable));
                break;
            }
            case Bin::kDebugLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(header, dataChunk, outData->m_debugLineInfos));
                break;
            }
            case Bin::kDebugAdjustedLineInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(header, dataChunk, outData->m_debugAdjustedLineInfos));
                break;
            }
            case Bin::kDebugSourceInfoFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(header, dataChunk, outData->m_debugSourceInfos));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kDebugSourceLocRunFourCc):
            case Bin::kDebugSourceLocRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(header, dataChunk, outData->m_debugSourceLocRuns));
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return SLANG_OK;
}

/* static */Result IRSerialReader::readStream(Stream* stream, IRSerialData* outData)
{
    typedef IRSerialBinary Bin;

    outData->clear();

    // Read the riff file
    RiffContainer container;
    SLANG_RETURN_ON_FAIL(RiffUtil::read(stream, container));

    // Find the riff module
    RiffContainer::ListChunk* module = container.getRoot()->findListRec(Bin::kSlangModuleFourCc);
    if (!module)
    {
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(readContainer(module, outData));
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

// TODO: The following function isn't really part of the IR serialization system, but rather
// a layered "container" format, and as such probably belongs in a higher-level system that
// simply calls into the `IRSerialReader` rather than being part of it...
//
/* static */Result IRSerialReader::readStreamModules(Stream* stream, Session* session, SourceManager* sourceManager, List<RefPtr<IRModule>>& outModules, List<FrontEndCompileRequest::ExtraEntryPointInfo>& outEntryPoints)
{
    // Load up the module
    RiffContainer container;
    SLANG_RETURN_ON_FAIL(RiffUtil::read(stream, container));
    
    List<RiffContainer::ListChunk*> moduleChunks;
    List<RiffContainer::DataChunk*> entryPointChunks;
    // First try to find a list
    {
        RiffContainer::ListChunk* listChunk = container.getRoot()->findListRec(IRSerialBinary::kSlangModuleListFourCc);
        if (listChunk)
        {
            listChunk->findContained(IRSerialBinary::kSlangModuleFourCc, moduleChunks);
            listChunk->findContained(IRSerialBinary::kEntryPointFourCc, entryPointChunks);
        }
        else
        {
            // Maybe its just a single module
            RiffContainer::ListChunk* moduleChunk = container.getRoot()->findListRec(IRSerialBinary::kSlangModuleFourCc);
            if (!moduleChunk)
            {
                // Couldn't find any modules
                return SLANG_FAIL;
            }
            moduleChunks.add(moduleChunk);
        }
    }

    // Okay, we need to decode into ir modules
    for (auto moduleChunk : moduleChunks)
    {
        IRSerialData serialData;

        SLANG_RETURN_ON_FAIL(IRSerialReader::readContainer(moduleChunk, &serialData));

        // Construct into a module
        RefPtr<IRModule> irModule;
        IRSerialReader reader;
        SLANG_RETURN_ON_FAIL(reader.read(serialData, session, sourceManager, irModule));

        outModules.add(irModule);
    }

    for( auto entryPointChunk : entryPointChunks )
    {
        auto reader = entryPointChunk->asReadHelper();

        auto readString = [&]()
        {
            uint32_t length = 0;
            reader.read(length);

            char* begin = (char*) reader.getData();
            reader.skip(length+1);

            return UnownedStringSlice(begin, begin + length);
        };

        FrontEndCompileRequest::ExtraEntryPointInfo entryPointInfo;

        entryPointInfo.name = session->getNamePool()->getName(readString());
        reader.read(entryPointInfo.profile);
        entryPointInfo.mangledName = readString();

        outEntryPoints.add(entryPointInfo);
    }

    return SLANG_OK;
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

        if (_isConstant(op))
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
                        
                    const size_t sliceSize = slice.getLength();
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
        else if (_isTextureTypeBase(op))
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

            auto dstOperands = dstInst->getOperands();

            for (int j = 0; j < numOperands; j++)
            {
                dstOperands[j].init(dstInst, insts[int(srcOperandIndices[j])]);
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

/* static */SlangResult IRSerialUtil::verifySerialize(IRModule* module, Session* session, SourceManager* sourceManager, IRSerialCompressionType compressionType, IRSerialWriter::OptionFlags optionFlags)
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
    OwnedMemoryStream memoryStream(FileAccess::ReadWrite);
    SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, compressionType, &memoryStream));

    // Reset stream
    memoryStream.seek(SeekOrigin::Start, 0);

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

} // namespace Slang
